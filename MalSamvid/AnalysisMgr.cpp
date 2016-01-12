#include "stdafx.h"

HANDLE	ghMutexVMWare;
HANDLE	ghMutexVbox;
HANDLE  hThreadArray[MAX_THREADS]; 

unsigned int CreateSyncObjects()
{
	// Create mutex for vmware thread
	ghMutexVMWare = CreateMutex(NULL, FALSE, NULL);	
	if (ghMutexVMWare == NULL) 
    {
        LogError(onlyFile, "CreateMutex ghMutexVMWare", GetLastError());
        return 0;
    }
	
	// Create mutex for vmware thread
	ghMutexVbox = CreateMutex(NULL, FALSE, NULL);
	if (ghMutexVbox == NULL) 
    {
        LogError(onlyFile, "CreateMutex ghMutexVbox", GetLastError());
        return 0;
    }

	return 1;
}