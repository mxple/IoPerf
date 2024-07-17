#include "IoPerf.h"

//
//  Globals
//

PFLT_FILTER     g_minifilterHandle;
PFLT_PORT       g_serverPort;
PFLT_PORT       g_clientPort;

RECORD_BUFFER   g_outputBuffer = { 0 };
KSPIN_LOCK      g_outputBufferLock;

LONG                    g_recordsAllocated;
NPAGED_LOOKASIDE_LIST   g_freeBufferList;

/*************************************************************************
    Function Definitions
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    UNREFERENCED_PARAMETER( RegistryPath );

    PSECURITY_DESCRIPTOR sd;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING uniString;
    NTSTATUS status = STATUS_SUCCESS;

    try 
    {
        KeInitializeSpinLock(&g_outputBufferLock);

        ExInitializeNPagedLookasideList( &g_freeBufferList,
										 NULL,
										 NULL,
										 MAX_RECORDS_ALLOCATE,
										 sizeof(RECORD),
										 'fPoI',
										 0 );

		status = FltRegisterFilter( DriverObject,
									&g_filterRegistration,
									&g_minifilterHandle );

        if (!NT_SUCCESS(status)) 
        {
            leave;
        }

		status = FltBuildDefaultSecurityDescriptor( &sd,
			                                        FLT_PORT_ALL_ACCESS );

		if (!NT_SUCCESS(status)) 
        {
			leave;
		}

		RtlInitUnicodeString(&uniString, IOPERF_PORT_NAME);

		InitializeObjectAttributes( &oa,
									&uniString,
									OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
									NULL,
									sd );

		status = FltCreateCommunicationPort( g_minifilterHandle,
											 &g_serverPort,
											 &oa,
											 NULL,
											 IoPerfConnect,
											 IoPerfDisconnect,
											 IoPerfMessage,
											 1 );

		FltFreeSecurityDescriptor( sd );

		if (!NT_SUCCESS(status)) 
        {
			leave;
		}

		//
		//  We are now ready to start filtering
		//

		status = FltStartFiltering( g_minifilterHandle );

	}
	finally 
    {
		if (!NT_SUCCESS(status)) 
        {
			if (NULL != g_serverPort) 
            {
				FltCloseCommunicationPort(g_serverPort);
			}

			if (NULL != g_minifilterHandle) 
            {
				FltUnregisterFilter(g_minifilterHandle);
			}

            ExDeleteNPagedLookasideList(&g_freeBufferList);
		}
	}

	return status;
}

NTSTATUS
IoPerfConnect(
    _In_ PFLT_PORT ClientPort,
    _In_ PVOID ServerPortCookie,
    _In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Flt_ConnectionCookie_Outptr_ PVOID* ConnectionCookie
	)
{
    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);
    UNREFERENCED_PARAMETER(ConnectionCookie);

    // Only allow one client connection 
    FLT_ASSERT(g_clientPort == NULL);
    g_clientPort = ClientPort;
    return STATUS_SUCCESS;
}

VOID
IoPerfDisconnect(
    _In_opt_ PVOID ConnectionCookie
	)
{
    UNREFERENCED_PARAMETER(ConnectionCookie);

    FltCloseClientPort(g_minifilterHandle, &g_clientPort);
}

NTSTATUS
IoPerfMessage(
    _In_ PVOID ConnectionCookie,
    _In_reads_bytes_opt_(InputBufferSize) PVOID InputBuffer,
    _In_ ULONG InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferSize,
    _Out_ PULONG ReturnOutputBufferLength
	)
{
    UNREFERENCED_PARAMETER(ConnectionCookie);
    UNREFERENCED_PARAMETER(InputBufferSize);

    IOPERF_COMMAND command;
    NTSTATUS status;

    if (InputBuffer == NULL) 
    {
        return STATUS_INVALID_PARAMETER;
    }

    // UM buffer access must have try-except guards
	try 
    {
		// Probe and capture input message
		command = ((PCOMMAND_MESSAGE)InputBuffer)->command;
	} 
    except(ExceptionFilter(GetExceptionInformation(), TRUE)) 
    {
		return GetExceptionCode();
	}

	switch (command) 
    {
	case GetIoPerfRecords:

		//
		//  Return as many records as can fit into the OutputBuffer
		//

		if ((OutputBuffer == NULL) || (OutputBufferSize == 0)) 
        {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

        //  NO 32-bit SUPPORT
		if (!IS_ALIGNED(OutputBuffer, sizeof(PVOID))) 
        {
			status = STATUS_DATATYPE_MISALIGNMENT;
			break;
		}

        //  Fill output buffer
		status = IoPerfFillOutput( OutputBuffer,
								   OutputBufferSize,
								   ReturnOutputBufferLength );

		break;

	default:
		status = STATUS_INVALID_PARAMETER;
		break;
	}

    return status;
}

NTSTATUS
InstanceUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Flags );

    FltCloseCommunicationPort( g_serverPort );
	FltUnregisterFilter( g_minifilterHandle );

    ExDeleteNPagedLookasideList(&g_freeBufferList);

    return STATUS_SUCCESS;
}

NTSTATUS 
InstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS  FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS  Flags,
    _In_ DEVICE_TYPE  VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE  VolumeFilesystemType
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    //
    // This is called to see if a filter would like to attach an instance to the given volume.
    //

    return STATUS_SUCCESS;
}

NTSTATUS
InstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS
IoPerfPreOperationCallback (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
    ) 
{
    UNREFERENCED_PARAMETER( FltObjects );

    ULONG status;

    //  Get a new record slot from the global buffer
    PRECORD record = allocRecord(&status);
    
    if (status != RECORD_TYPE_NORMAL)
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    //  Save info into record
    record->majorId = Data->Iopb->MajorFunction;
    record->minorId = Data->Iopb->MinorFunction;
    record->pid     = (ULONG_PTR)PsGetCurrentProcessId();

    KeQuerySystemTimePrecise((PLARGE_INTEGER) &record->timeTaken);

	*CompletionContext = record;
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
};

FLT_POSTOP_CALLBACK_STATUS
IoPerfPostOperationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS flags
    ) 
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( flags );

    PRECORD record = (PRECORD)CompletionContext;
    ULONG64 endTime;

    //  Save info into record
    record->status = Data->IoStatus.Status;

    KeQuerySystemTimePrecise((PLARGE_INTEGER) &endTime);
    record->timeTaken = endTime - record->timeTaken;

    //  TODO: support transactions?

    //  Done with this record
    outputPushRecord(record);

    freeRecord(record);

    return FLT_POSTOP_FINISHED_PROCESSING;
};
