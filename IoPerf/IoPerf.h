#pragma once

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <intsafe.h>

#include "../shared.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

/************************************************************************
    Defines
*************************************************************************/

#define MAX_RECORDS_ALLOCATE 512

#define RECORD_TYPE_NORMAL                      0x00000000
#define RECORD_TYPE_EXCEED_MEMORY_ALLOWANCE     0x20000000
#define RECORD_TYPE_OUT_OF_MEMORY               0x10000000

/************************************************************************
    Globals
*************************************************************************/

extern PFLT_FILTER      g_minifilterHandle;

extern PFLT_PORT        g_serverPort;
extern PFLT_PORT        g_clientPort;


extern RECORD_BUFFER    g_outputBuffer;
extern KSPIN_LOCK       g_outputBufferLock;

extern __volatile LONG          g_recordsAllocated;
extern NPAGED_LOOKASIDE_LIST    g_freeBufferList;

CONST extern FLT_REGISTRATION g_filterRegistration;

/*************************************************************************
    Prototypes
*************************************************************************/

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
InstanceUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS 
InstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS  FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS  Flags,
    _In_ DEVICE_TYPE  VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE  VolumeFilesystemTypetime
    );

NTSTATUS
InstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

NTSTATUS
IoPerfMessage(
    _In_ PVOID ConnectionCookie,
    _In_reads_bytes_opt_(InputBufferSize) PVOID InputBuffer,
    _In_ ULONG InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferSize,
    _Out_ PULONG ReturnOutputBufferLength
	);

NTSTATUS
IoPerfConnect(
    _In_ PFLT_PORT ClientPort,
    _In_ PVOID ServerPortCookie,
    _In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Flt_ConnectionCookie_Outptr_ PVOID* ConnectionCookie
	);

VOID
IoPerfDisconnect(
    _In_opt_ PVOID ConnectionCookie
	);

FLT_PREOP_CALLBACK_STATUS
IoPerfPreOperationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FtlObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
IoPerfPostOperationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS flags
    );

/*************************************************************************
    Library Function Prototypes 
*************************************************************************/

PRECORD
getNewRecord();

VOID
outputPushRecord(PRECORD record);

NTSTATUS
IoPerfFillOutput(
    _Out_writes_bytes_to_(OutputBufferLength, *ReturnOutputBufferLength) PUCHAR OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
    );


LONG
ExceptionFilter(
    _In_ PEXCEPTION_POINTERS ExceptionPointer,
    _In_ BOOLEAN AccessingUserBuffer
	);

PRECORD
allocRecord(
    _Out_ PULONG recordType 
    );

VOID
freeRecord(
    _In_ PRECORD record
    );
