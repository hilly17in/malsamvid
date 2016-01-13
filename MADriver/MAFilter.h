# pragma once

#pragma warning (error: 4047)

//
//  Name of PEC Filter's Device Object
//
#define MAFILTER_DEVICE_NAME				  L"\\Device\\MAFilterDevice"


NTSTATUS InitDriver(PDRIVER_OBJECT) ;

VOID	 CleanDriver(); 

NTSTATUS RegisterMAFilter() ;

VOID UnRegisterMAFilter() ;

VOID Clean_gPidList() ;