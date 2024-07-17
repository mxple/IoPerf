#include "IoPerf.h"

PRECORD
getNewRecord() {
	KIRQL oldirql;

	KeAcquireSpinLock(&g_outputBufferLock, &oldirql);

	PRECORD ret = &g_outputBuffer.buffer[g_outputBuffer.head];

	g_outputBuffer.head = (g_outputBuffer.head + 1) % OUTPUT_BUFFER_SZ;

	if (g_outputBuffer.head == g_outputBuffer.size)
	{
	//	DbgPrint("[ WARN ] Unused record overwritten.");

		g_outputBuffer.size = (g_outputBuffer.size + 1) % OUTPUT_BUFFER_SZ;
	}

    KeReleaseSpinLock(&g_outputBufferLock, oldirql);

	//	Can zero record here if so desired
	//  RtlFillMemory(ret, sizeof(RECORD), 0);

	return ret;
}

VOID
outputPushRecord(
	PRECORD record
	)
{ 
	KIRQL oldirql;

	KeAcquireSpinLock(&g_outputBufferLock, &oldirql);

	g_outputBuffer.buffer[g_outputBuffer.head] = *record;

	g_outputBuffer.head = (g_outputBuffer.head + 1) % OUTPUT_BUFFER_SZ;
	g_outputBuffer.size = max(g_outputBuffer.size, g_outputBuffer.head);

    KeReleaseSpinLock(&g_outputBufferLock, oldirql);
}

NTSTATUS
IoPerfFillOutput(
    _Out_writes_bytes_to_(OutputBufferLength, *ReturnOutputBufferLength) PUCHAR OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
	)
{
	NTSTATUS status = STATUS_NO_MORE_ENTRIES;
	ULONG bytesWritten = 0;
	KIRQL oldIrql;

	KeAcquireSpinLock(&g_outputBufferLock, &oldIrql);

	BOOLEAN recordsAvailable = g_outputBuffer.size > 0;
	UINT recordsToCopy = min(g_outputBuffer.size, OutputBufferLength / sizeof(RECORD));

	try 
	{
		RtlCopyMemory(OutputBuffer, g_outputBuffer.buffer, sizeof(RECORD) * recordsToCopy);
	}
	except (ExceptionFilter(GetExceptionInformation(), TRUE))
	{
		KeReleaseSpinLock(&g_outputBufferLock, oldIrql);
		return GetExceptionCode();
	}

	g_outputBuffer.size -= recordsToCopy;
	bytesWritten		+= sizeof(RECORD) * recordsToCopy;

	KeReleaseSpinLock(&g_outputBufferLock, oldIrql);

	if ((bytesWritten == 0) && recordsAvailable) 
	{
		status = STATUS_BUFFER_TOO_SMALL;
	}
	else if (bytesWritten > 0) 
	{
		status = STATUS_SUCCESS;
	}

	*ReturnOutputBufferLength = bytesWritten;

	return status;
}

LONG
ExceptionFilter(
	_In_ PEXCEPTION_POINTERS ExceptionPointer,
	_In_ BOOLEAN AccessingUserBuffer
	)
{
	NTSTATUS Status;

	Status = ExceptionPointer->ExceptionRecord->ExceptionCode;

	if (!FsRtlIsNtstatusExpected(Status) &&
		!AccessingUserBuffer) {

		return EXCEPTION_CONTINUE_SEARCH;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

PRECORD
allocRecord(
	_Out_ PULONG recordType
	)
{
	PVOID record;
	ULONG status = RECORD_TYPE_NORMAL;

	if (g_recordsAllocated < MAX_RECORDS_ALLOCATE) 
	{
		record = ExAllocateFromNPagedLookasideList(&g_freeBufferList);

		if (record != NULL) 
		{
			InterlockedIncrement(&g_recordsAllocated);
		}
		else
		{
			DbgPrint("Out of memory!\n");
			status = RECORD_TYPE_OUT_OF_MEMORY;
		}
	}
	else
	{
		DbgPrint("Exceeeded memory allowance!\n");
		status = RECORD_TYPE_EXCEED_MEMORY_ALLOWANCE;
		record = NULL;
	}

	*recordType = status;
	return record;
}

VOID
freeRecord(
	_In_ PRECORD record
	)
{
	InterlockedDecrement(&g_recordsAllocated);
	ExFreeToNPagedLookasideList(&g_freeBufferList, record);
}
