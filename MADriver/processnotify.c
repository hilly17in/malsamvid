# include "passthrough.h"

VOID ViewPidList();

BOOLEAN IsPidPresentinList(unsigned long pid)
{
	PLIST_ENTRY		pEntry ;

	if(IsListEmpty(&leProcessList))
		return FALSE;
	
	pEntry = leProcessList.Flink;

	while(pEntry != &leProcessList)
	{
		PPROCESS_LIST ptemp;

		ptemp = (PPROCESS_LIST)CONTAINING_RECORD(pEntry, PROCESS_LIST, ListEntry);
		if(ptemp == NULL)
			break;	

		if(pid == ptemp->Pid)
			return TRUE;


		pEntry = pEntry->Flink;
	}
	return FALSE;
}

VOID
CreateProcessNotifyRoutine (
    HANDLE  ParentId,
    HANDLE  ProcessId,
    BOOLEAN  Create
    )
{
	NTSTATUS status ;
	PPROCESS_LIST	proclist;


	DbgPrint("CreateProcesNotifyRoutine: ProcessId = %d, ParentID = %d, %d \n", 
				(unsigned int)ProcessId, (unsigned int)ParentId, Create) ;

	if(!gPid)
		return ;

	if( Create == FALSE )
	{
		if( IsPidPresentinList( (unsigned long)ProcessId ) )
		{
			NotifyProcessData pData ;
			RtlZeroMemory(&pData, sizeof(pData));

			pData.Pid = (unsigned int)ProcessId;
			pData.ParentId = (unsigned int)ParentId;
			pData.Create = Create;

			SendMessageToUserMode(NotifyProcess, &pData, 0, 0, NULL);
			DbgPrint("a malware process is killed -- pid = %d\n", (unsigned long)ProcessId);
			ViewPidList();
		}
		
		return ;
	}

	if(IsPidPresentinList((unsigned long)ParentId))
	{
		NotifyProcessData pData ;
		RtlZeroMemory(&pData, sizeof(pData));

		pData.Pid = (unsigned int)ProcessId;
		pData.ParentId = (unsigned int)ParentId;
		pData.Create = Create;
		SendMessageToUserMode(NotifyProcess, &pData, 0, 0, NULL);

		// Add this to our list also.
		proclist = (PPROCESS_LIST)ExAllocateFromNPagedLookasideList(&gPidList);
		if(proclist == NULL)
		{
			return;
		}

		DbgPrint("this pid = %d is added to our list \n", (unsigned long)ProcessId);
		proclist->Pid = (unsigned long)ProcessId;

		InsertTailList(&leProcessList, &proclist->ListEntry);

	}
	ViewPidList();

} // CreateProcessNotifyRoutine

VOID ViewPidList()
{
	PLIST_ENTRY		pEntry ;

	if(IsListEmpty(&leProcessList))
		return ;
	
	pEntry = leProcessList.Flink;

	while(pEntry != &leProcessList)
	{
		PPROCESS_LIST ptemp;

		ptemp = (PPROCESS_LIST)CONTAINING_RECORD(pEntry, PROCESS_LIST, ListEntry);
		if(ptemp == NULL)
			break;	

		DbgPrint("[--- %d ---]", ptemp->Pid);

		pEntry = pEntry->Flink;
	}
	DbgPrint("\n");

	return ;

}