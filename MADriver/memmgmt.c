# include "passthrough.h"

NTSTATUS 
MAllocateUnicodeString 
(
 PUNICODE_STRING String
 ) 
{ 
	PAGED_CODE(); 
	String->Buffer = ExAllocatePoolWithTag( NonPagedPool, 
											String->MaximumLength, 'tset' ); 
	
	if (String->Buffer == NULL) 
	{ 
		return STATUS_INSUFFICIENT_RESOURCES; 
	}
	
	String->Length = 0; 

	return STATUS_SUCCESS; 
} 


NTSTATUS
TerminateThisPid(unsigned long pid)
{
	OBJECT_ATTRIBUTES	ObjAttr;
	CLIENT_ID			ClientId;
	HANDLE				ProcessHandle ;
	NTSTATUS			status = STATUS_SUCCESS;


	RtlZeroMemory(&ClientId, sizeof(CLIENT_ID));

	RtlZeroMemory(&ObjAttr, sizeof(OBJECT_ATTRIBUTES));
	
	ClientId.UniqueProcess = (HANDLE)pid ;
	
	InitializeObjectAttributes(&ObjAttr, NULL, 0, NULL, NULL);
	
	status = ZwOpenProcess(&ProcessHandle, PROCESS_ALL_ACCESS, &ObjAttr, &ClientId);

	if(status == STATUS_SUCCESS)
			status = ZwTerminateProcess(ProcessHandle, 0);

	return status;

}


VOID
MATerminateProcess()
{
	NTSTATUS			status = STATUS_SUCCESS;
	PLIST_ENTRY			pEntry ;

	if(IsListEmpty(&leProcessList))
		return ;


	pEntry = leProcessList.Flink;

	while(pEntry != &leProcessList)
	{
		PPROCESS_LIST ptemp;

		ptemp = (PPROCESS_LIST)CONTAINING_RECORD(pEntry, PROCESS_LIST, ListEntry);
		if(ptemp == NULL)
			break;	

		if( ptemp->Pid != 0)
		{
			status = TerminateThisPid(ptemp->Pid);
			DbgPrint("could not terminate this process: %d \n", status);
		}

		pEntry = pEntry->Flink;

	} // while

	return ;
}
