# pragma once

extern PFLT_FILTER gFilterHandle;

extern PDRIVER_OBJECT gpDriverObject;

extern CONST FLT_REGISTRATION FilterRegistration ;

extern PFLT_PORT		gServerPort ;

extern PFLT_PORT		gClientPort ;

extern PFLT_PORT		gRegPort;

extern ULONG gTraceFlags;

extern ULONG_PTR		OperationStatusCtx ;

extern UNICODE_STRING			usProgramName;

extern unsigned long	gPid ;

extern unsigned long	cPid ;

extern unsigned long	sPid ;

extern NPAGED_LOOKASIDE_LIST  gPidList ;		// this is a look aside list for having pids

extern LIST_ENTRY		leProcessList;

extern REGISTRY_CONTEXT	RegContext ;