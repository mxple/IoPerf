#include "IoPerf.h"
//
//  Callback registration
//

//
//  Tells the compiler to define all following DATA and CONSTANT DATA to
//  be placed in the INIT segment.
//

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#pragma const_seg("INIT")
#endif

CONST FLT_OPERATION_REGISTRATION callbacks[] =
{
	{ IRP_MJ_CREATE,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_CLOSE,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_READ,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_WRITE,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_QUERY_INFORMATION,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_SET_INFORMATION,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_QUERY_EA,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_SET_EA,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_FLUSH_BUFFERS,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_QUERY_VOLUME_INFORMATION,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_SET_VOLUME_INFORMATION,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_DIRECTORY_CONTROL,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_FILE_SYSTEM_CONTROL,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_DEVICE_CONTROL,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_INTERNAL_DEVICE_CONTROL,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_LOCK_CONTROL,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_CLEANUP,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_CREATE_MAILSLOT,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_QUERY_SECURITY,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_SET_SECURITY,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_QUERY_QUOTA,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_SET_QUOTA,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_PNP,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_ACQUIRE_FOR_MOD_WRITE,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_RELEASE_FOR_MOD_WRITE,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_ACQUIRE_FOR_CC_FLUSH,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

    { IRP_MJ_RELEASE_FOR_CC_FLUSH,
      0,
      IoPerfPreOperationCallback,
      IoPerfPostOperationCallback },

	{ IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE,
	  0,
	  IoPerfPreOperationCallback,
	  IoPerfPostOperationCallback },

    { IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION g_filterRegistration = 
{
    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags
    NULL,                               //  Context
    callbacks,                          //  Operation callbacks
    InstanceUnload,                     //  FilterUnload
    InstanceSetupCallback,              //  InstanceSetup
    InstanceQueryTeardown,              //  InstanceQueryTeardown
    NULL,                               //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete
    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent
};
