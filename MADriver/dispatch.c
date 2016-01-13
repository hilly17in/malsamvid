# include "passthrough.h"

# define MAX_PROGRAM_LEN	16

NTSTATUS 
MADispatchCreate
(
 PDEVICE_OBJECT pDevObject, 
 PIRP pIrp
) 
{
	NTSTATUS	NtStatus = STATUS_SUCCESS ;

	pIrp->IoStatus.Status = NtStatus ;
	pIrp->IoStatus.Information = 0 ;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT) ; 
	
	return NtStatus ;

}

NTSTATUS 
MADispatchClose
(
 PDEVICE_OBJECT pDevObj, 
 PIRP pIrp
)
{
	NTSTATUS NtStatus = STATUS_SUCCESS ;

	pIrp->IoStatus.Status = STATUS_SUCCESS ;
	pIrp->IoStatus.Information = 0 ;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT) ;
  	
	return NtStatus ;
}

NTSTATUS 
MADispatchIoctl
(
 PDEVICE_OBJECT pDevObj, 
 PIRP pIrp
 ) 
{	
	NTSTATUS				status = STATUS_SUCCESS ;
	PIO_STACK_LOCATION		pIoStackIrp ;
	ULONG					code ;
	ULONG					in, out ;
	PVOID					pBuff ;
	PPROCESS_LIST			proclist;

	PAGED_CODE() ;

	pIoStackIrp = IoGetCurrentIrpStackLocation(pIrp) ;

	if (pIoStackIrp)
	{
		pBuff = (PVOID)pIrp->AssociatedIrp.SystemBuffer ;
		in = pIoStackIrp->Parameters.DeviceIoControl.InputBufferLength ;
		out = pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength ;

		code = pIoStackIrp->Parameters.DeviceIoControl.IoControlCode ;

		switch(code)
		{
		case IOCTL_SEND_AND_INITIALIZE:

			if(pBuff != NULL)
			{
				RtlCopyMemory(&gPid, pBuff, sizeof(unsigned long)) ;

				proclist = (PPROCESS_LIST)ExAllocateFromNPagedLookasideList(&gPidList);
				if(proclist == NULL)
				{
					break;
				}
				
				proclist->Pid = gPid;
					
				InsertTailList(&leProcessList, &proclist->ListEntry);

				DbgPrint("Program to be analyzed is %d\n", gPid);

			}
			else
			{
				DbgPrint("PID (to be analyzed) recieved is NULL \n");
			}

			

			break ;

		case IOCTL_SEND_SERVICE_PID:

			if(pBuff != NULL)
			{
				RtlCopyMemory(&sPid, pBuff, sizeof(unsigned long));
				DbgPrint("PID - services.exe to be analyzed is %d\n", gPid);
			}
			else
			{
				DbgPrint("PID - services.exe to be analyzed is NULL \n");
			}

			break;

		case IOCTL_TERMINATE_PROCESSID:
			
			//RtlCopyMemory(&gPid, pBuff, sizeof(unsigned int)) ;

			DbgPrint("PID to terminate is %d \n", gPid);
			
			MATerminateProcess();

			break ;
		} // switch
	}	// if

	pIrp->IoStatus.Status = status ;
	pIrp->IoStatus.Information = 0 ;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT) ;

	return status ;
}
