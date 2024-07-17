#include "UMapp.hpp"

#define POLL_INTERVAL 1500  // In ms

typedef struct _APP_CONTEXT {
    HANDLE port;
    HANDLE semaphore;
    BOOLEAN exiting;

    std::unordered_map<ULONG_PTR, ULONG64> data;
    HANDLE dataMutex;
} APP_CONTEXT, *PAPP_CONTEXT;

DWORD
WINAPI
RetrieveLogRecords(
    _In_ LPVOID lpParameter
	)
{
    APP_CONTEXT *context = (APP_CONTEXT*)lpParameter;
    RECORD buffer[OUTPUT_BUFFER_SZ];
    DWORD bytesReturned = 0;
    DWORD used;
    HRESULT hResult;
    COMMAND_MESSAGE commandMessage;

    printf("Log: Starting up\n");

    while (TRUE) 
    {
        if (context->exiting) 
        {
            break;
        }

        commandMessage.command = GetIoPerfRecords;

        hResult = FilterSendMessage( context->port,
									 &commandMessage,
									 sizeof(COMMAND_MESSAGE),
									 buffer,
									 sizeof(buffer),
									 &bytesReturned );

        if (IS_ERROR(hResult)) 
        {
            if (hResult == HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)) 
            {
                printf("The kernel component of minispy has unloaded. Exiting\n");
                ExitProcess(0);
            }

            if (hResult != HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) 
			{
				printf("UNEXPECTED ERROR received: %x\n", hResult);
			}

			Sleep(POLL_INTERVAL);
            continue;
        }

        WaitForSingleObject(context->dataMutex, INFINITE);

        UINT numReturned = bytesReturned / sizeof(RECORD);
        for (unsigned int i = 0; i < numReturned; i++) 
        {
            PRECORD curr = &buffer[i];

			context->data[curr->pid] += curr->timeTaken;
        }

        printData(context->data);

        ReleaseMutex(context->dataMutex);

        if (bytesReturned == 0) 
        {
            Sleep(POLL_INTERVAL);
        }
    }

    ReleaseSemaphore(context->semaphore, 1, NULL);
    return 0;
}

int _cdecl
main(
    _In_ int argc,
    _In_reads_(argc) char* argv[]
)
{
    HANDLE port = INVALID_HANDLE_VALUE;
    HANDLE thread = NULL;
    ULONG threadId;
    HRESULT hResult = S_OK;
    DWORD result;
    APP_CONTEXT context;
    CHAR inputChar;

    printf("Connecting to filter's port...\n");

    hResult = FilterConnectCommunicationPort(   IOPERF_PORT_NAME,
												0,
												NULL,
												0,
												NULL,
												&port );

    if (IS_ERROR(hResult)) 
    {
        printf("Could not connect to filter: 0x%08x\n", hResult);
        goto MAIN_EXIT;
    }

    //  Init context
    context.port = port;
    context.exiting = false;

    context.semaphore = CreateSemaphore(NULL, 0, 1, L"IoPerf Shut Down");

    if (context.semaphore == NULL) 
    {
        result = GetLastError();
		printf("Could not create semaphore: %d\n", result);
        goto MAIN_EXIT;
    }

    context.dataMutex = CreateMutex(NULL, FALSE, NULL);

    if (context.dataMutex == NULL) 
    {
        result = GetLastError();
		printf("Could not create mutex: %d\n", result);
        goto MAIN_EXIT;
    }

    printf("Creating logging thread...\n");

    thread = CreateThread(  NULL,
							0,
							RetrieveLogRecords,
							(LPVOID)&context,
							0,
							&threadId );

    if (!thread) 
    {
        result = GetLastError();
        printf("Could not create logging thread: %d\n", result);
        goto MAIN_EXIT;
    }

    while (TRUE) 
    {
        //WaitForSingleObject(context.dataMutex, INFINITE);

        //auto mapCopy = context.data;

        //ReleaseMutex(context.dataMutex);

        //printData(mapCopy);

        Sleep(200);
    }

//MAIN_CLEANUP:
    //  Wait for thread
    context.exiting = TRUE;

    WaitForSingleObject(context.semaphore, INFINITE);
    
    //  Close files if needed

MAIN_EXIT:

    if (context.semaphore)
    {
        CloseHandle(context.semaphore);
    }

    if (context.dataMutex)
    {
        CloseHandle(context.dataMutex);
    }

    if (thread) 
    {
        CloseHandle(thread);
    }

    if (port != INVALID_HANDLE_VALUE) 
    {
        CloseHandle(port);
    }

    while (inputChar = (CHAR)getchar())
    {
        if (inputChar == 'q')
        {
            break;
        }
    }
    
    return 0;
}
