# pragma once

VOID
CreateProcessNotifyRoutine (
    HANDLE  ParentId,
    HANDLE  ProcessId,
    BOOLEAN  Create
    ) ;

typedef struct _PROCESS_LIST
{
	LIST_ENTRY	ListEntry;
	unsigned long Pid;
} PROCESS_LIST, *PPROCESS_LIST;

BOOLEAN IsPidPresentinList
(
 unsigned long pid
 );
