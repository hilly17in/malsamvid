# include "passthrough.h"

PFLT_FILTER gFilterHandle;

PDRIVER_OBJECT gpDriverObject;

//
//  operation registration
//
CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE,
      0,
      PtPreCreate,
      PtPostCreate },

    { IRP_MJ_READ,
      0,
      PtPreRead,
      PtPostRead },

    { IRP_MJ_WRITE,
      0,
      PtPreWrite,
      PtPostWrite },

    { IRP_MJ_SET_INFORMATION,
      0,
      PtPreSetInformation,
      PtPostSetInformation },

    { IRP_MJ_DEVICE_CONTROL,
      0,
      PtPreDeviceControl,
      PtPostDeviceControl },

    { IRP_MJ_SET_SECURITY,
      0,
      PtPreSetSecurity,
      PtPostSetSecurity },

	{ IRP_MJ_OPERATION_END }
};

//
//  This defines what we want to filter with FltMgr
//
CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    PtUnload,                           //  MiniFilterUnload

    PtInstanceSetup,                    //  InstanceSetup
    PtInstanceQueryTeardown,            //  InstanceQueryTeardown
    PtInstanceTeardownStart,            //  InstanceTeardownStart
    PtInstanceTeardownComplete,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};

PFLT_PORT		gServerPort ;

PFLT_PORT		gClientPort ;

PFLT_PORT		gRegPort;

ULONG gTraceFlags = 1 ;

ULONG_PTR		OperationStatusCtx = 1;

UNICODE_STRING			usProgramName;

unsigned long	gPid = 0;
unsigned long	cPid = 0;
unsigned long	sPid = 0;

NPAGED_LOOKASIDE_LIST  gPidList ;		// this is a look aside list for having pids

LIST_ENTRY		leProcessList;

REGISTRY_CONTEXT	RegContext ;