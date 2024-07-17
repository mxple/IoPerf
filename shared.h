#include <intsafe.h>

#define IOPERF_PORT_NAME L"\\IoPerfPort"
#define OUTPUT_BUFFER_SZ 1024

typedef struct _RECORD {
    ULONG64     timeTaken;
    ULONG_PTR   pid;

    NTSTATUS    status;

    UCHAR       majorId;
    UCHAR       minorId;

    UCHAR       completed;
} RECORD, *PRECORD;


typedef enum _IOPERF_COMMAND {
    GetIoPerfRecords,
} IOPERF_COMMAND;

typedef struct _COMMAND_MESSAGE {
    IOPERF_COMMAND command;
    ULONG reserved;  // Alignment
    //  Add dynamic data member if needed
} COMMAND_MESSAGE, * PCOMMAND_MESSAGE;

typedef struct _RECORD_BUFFER {
    RECORD  buffer[OUTPUT_BUFFER_SZ];
    DWORD   head;
    DWORD   size;
} RECORD_BUFFER;
