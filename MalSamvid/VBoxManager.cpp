#include "StdAfx.h"

BSTR WSysAllocString(const char *psz)
{
	wchar_t *wpsz;
	int requiredBufferSizeByte=0;
	int maxlen=0;

	if( (psz == NULL) || (strlen(psz) == 0) )
		return NULL;

	requiredBufferSizeByte = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);

	//printf("required buffer size bytes = %d \n", requiredBufferSizeByte);

	maxlen = requiredBufferSizeByte + 1 ;
	wpsz = (wchar_t *)malloc(maxlen);
	if(wpsz == NULL)
		return NULL;

	int err = MultiByteToWideChar(CP_ACP, 0, psz, -1, wpsz, maxlen);
	if (err == 0)
		printf("Error With MultiByteToWideChar: %d", GetLastError());

	return SysAllocString(wpsz);
}

unsigned int GetLastErrorInfo(char **)
{
	return 0;
}


CVBoxManager::CVBoxManager(void)
{
}

CVBoxManager::~CVBoxManager(void)
{
	SAFE_RELEASE(iGuestSession);
    SAFE_RELEASE(iConsole);
    SAFE_RELEASE(iSession);
	SAFE_RELEASE(iMachine);
}

unsigned int CVBoxManager::Init()
{
	unsigned int retVal = MVID_OK;

	HRESULT rc;
	
	/* Initialize the COM subsystem. */

	CoInitialize(NULL);


	/* Instantiate the VirtualBox root object. */

	rc = CoCreateInstance(CLSID_VirtualBox,				/* the VirtualBox base object */
 
		NULL,                   /* no aggregation */

		CLSCTX_LOCAL_SERVER,    /* the object lives in a server process on this machine */

		IID_IVirtualBox,        /* IID of the interface */

		(void**)&iVirtualBox);


	if (!SUCCEEDED(rc))

	{

		printf("Error creating VirtualBox instance! rc = 0x%x\n", rc);

		retVal = ERRVBOX_CREATE_VBOX_INSTANCE;

	}

	
	return retVal;
}

void CVBoxManager::Release()
{
	SAFE_RELEASE(iGuestSession);
    SAFE_RELEASE(iConsole);
    SAFE_RELEASE(iSession);
	SAFE_RELEASE(iMachine);

	iVirtualBox->Release();

}


unsigned int CVBoxManager::FindMachine(BSTR bsMachineName)
{
	HRESULT rc;
	unsigned int retVal = MVID_OK;

	rc = iVirtualBox->FindMachine(bsMachineName, &iMachine);

	if (!SUCCEEDED(rc))
	{
		printf("Error finding the specified Virtual Machine Name rc = 0x%x\n", rc);

		retVal = ERRVBOX_COULD_NOT_FIND_VM;

		goto cleanup;
	}

	HRESULT rc1;
	rc1 = iMachine->get_Id(&guid);

	if (!SUCCEEDED(rc1))
	{
		printf("Error finding the specified Virtual Machine Name rc = 0x%x\n", rc1);

		retVal = ERRVBOX_COULD_NOT_GET_VM_ID;

		goto cleanup;
	}

	
	retVal = this->GetSession();

cleanup:
	return retVal;

}


unsigned int CVBoxManager::GetSession()
{
	unsigned int retVal = MVID_OK;
	HRESULT rc;

    
	/* Create the session object. */

	rc = CoCreateInstance(CLSID_Session,        /* the VirtualBox base object */

		NULL,                 /* no aggregation */

		CLSCTX_INPROC_SERVER, /* the object lives in a server process on this machine */

		IID_ISession,         /* IID of the interface */

		(void**)&iSession);

	if (!SUCCEEDED(rc))
	{
		//LogErrorVbox(BothFilePrint, "CoCreateInstance-Session", rc);
		printf("guestsession-cocreateinstance error: %x \n", rc);
		retVal = ERRVBOX_CREATE_SESSION_INSTANCE;
	}

	return retVal;
}

unsigned int CVBoxManager::LaunchVM()
{
	unsigned int retVal = MVID_OK;

	HRESULT rc;

	IProgress *iProgress = NULL;
	
	// TODO
	// must validate iSession


	// TODO
	// make it user argument
	BSTR sessiontype = SysAllocString(L"gui");
	
	rc = iMachine->LaunchVMProcess(iSession, 
							 sessiontype,
							 NULL, 
							 &iProgress);

	if (!SUCCEEDED(rc))
	{
		LogErrorVbox(BothFilePrint, "LaunchVMProcess", rc);//printf("Could not open remote session! rc = 0x%x\n", rc);
        retVal = ERRVBOX_LAUNCH_VM_FAILED ;   
		goto cleanup;
	}

	rc = iProgress->WaitForCompletion(-1);

	rc = iSession->get_Console(&iConsole);
	if (!SUCCEEDED(rc))
	{
		LogErrorVbox(BothFilePrint, "iSession->get_console", rc);//printf("Could not open remote session! rc = 0x%x\n", rc);
        retVal = ERRVBOX_GET_CONSOLE_FAILED ;   
		goto cleanup;
	}

cleanup:

	SAFE_RELEASE(iProgress);
	SysFreeString(sessiontype);
 
	return retVal;
}

unsigned int CVBoxManager::GetGuest()
{
	unsigned int retVal = MVID_OK;
	HRESULT rc;

	rc = iConsole->get_Guest(&iGuest);
	if(!SUCCEEDED(rc))
	{
		retVal = ERRVBOX_CREATE_GUEST_FAILED;
	}

	return retVal;
}


unsigned int CVBoxManager::CreateGuestSession(BSTR aUser, BSTR aPass)
{
	unsigned int retVal;

	HRESULT rc;


	retVal = GetGuest();
	if(retVal != MVID_OK)
		goto cleanup;

	rc = iGuest->CreateSession(aUser, aPass, NULL, NULL, &iGuestSession);
	if(!SUCCEEDED(rc))
	{
		retVal = ERRVBOX_CREATE_GUESTSESSION_FAILED;
	}



cleanup:
	return retVal;
}


void CVBoxManager::SetExtendedLastCOMError(HRESULT rc)
{
	// todo
}

unsigned int CVBoxManager::GetExtendedLastCOMError()
{
	//todo
	return 0;
}

unsigned int CVBoxManager::DirectoryExists(BSTR bsDirPath, BOOL *bExist)
{
	unsigned int retVal = MVID_OK;
	
	BOOL aExists;
	HRESULT rc;
	unsigned int ucnt=0;
	while(ucnt < 100)
	{
		rc = iGuestSession->DirectoryExists(bsDirPath, FALSE, &aExists);
		if(SUCCEEDED(rc))
		{
			printf("here... setting %d", aExists);
			*bExist = aExists ;
			retVal = MVID_OK;
			break;
		}
		else {
			SetExtendedLastCOMError(rc);
			Sleep(1000);
			retVal = ERRVBOX_DIREXISTS_CHECK_ERROR;
			ucnt++;
		}
	}

	return retVal;
}

unsigned int CVBoxManager::CopyFileToVM(BSTR aSource, BSTR aDest)
{
	unsigned int retVal = MVID_OK;
	IProgress *aProgress;
	HRESULT		rc;
	HRESULT		rp;
	

	rc = iGuestSession->FileCopyToGuest(aSource, aDest, 0, &aProgress);
	// check for failure and exit
	if(FAILED(rc))
	{
		// SetExtendedErrorReport()
		retVal = ERRVBOX_COPYTO_FAILED;
		goto cleanup;
	}

	rp = aProgress->WaitForCompletion(-1);
	// check for failure and exit
	if(FAILED(rp))
	{
		// SetExtendedErrorReport()
		retVal = ERRVBOX_WAITCOMPLETION_FAILED;
		goto cleanup;
	}

	// return success;
cleanup:
	return retVal;
	
}

unsigned int CVBoxManager::CopyFileFromVM(BSTR aSrcGuest, BSTR aDestHost)
{

	unsigned int retVal = MVID_OK;
	IProgress  *aProgress;
	HRESULT		rc;
	HRESULT		rp;

	rc = iGuestSession->FileCopyFromGuest(aSrcGuest, aDestHost, 0, &aProgress);
	// check for failure and exit
	if(FAILED(rc))
	{
		// SetExtendedErrorReport()
		retVal = ERRVBOX_COPYFROM_FAILED;
		goto cleanup;
	}

	rp = aProgress->WaitForCompletion(-1);
	// check for failure and exit
	if(FAILED(rp))
	{
		// SetExtendedErrorReport()
		retVal = ERRVBOX_WAITCOMPLETION_FAILED;
		goto cleanup;
	}


cleanup:
	return retVal;
}

unsigned int CVBoxManager::RunProgram(BSTR bPath, BSTR bArg1, ULONG *uPid)
{
	IGuestProcess *aGuestProcess;
	HRESULT rc;
	HRESULT rp;
	unsigned int retVal = MVID_OK;
	ProcessWaitResult aReason;
	ULONG	aPid;
	

	// build the args
	SAFEARRAY	*aArgs;
	aArgs = SafeArrayCreateVector(VT_BSTR, 0, 1);
	long index;
	index = 0; 
	//if(bArg1 != NULL) {

	//	rc = SafeArrayPutElement(aArgs, &index, &bArg1);
	//	if(FAILED(rc))
	//	{
	//		printf("safe array could not be created : %x\n", rc);
	//	}

	//}
	//else {
	//
	//	retVal = ERRVBOX_INVALID_ARGS;
	//	//LogErrorVbox(BothFilePrint, "CVBoxManager::RunProgram", retVal);
	//	printf("runprogram failed. %x\n", rc);
	//	goto cleanup;
	//}

	//printf("Analyzer Path = %S \n Sample Path = %S\n", bPath, bArg1);
	printf("Analyzer Path = %S \n ", bPath);
	rc = iGuestSession->ProcessCreate(bPath, NULL, NULL, 0, 0, &aGuestProcess);
	if(FAILED(rc))
	{
		retVal = ERRVBOX_PROCESSCREATE_FAILED;	
		//LogErrorVbox(onlyFile, "aguestprocess->processcreate", rc);
		goto cleanup;
	}

	rp = aGuestProcess->WaitFor(ProcessWaitForFlag_Start, 0, &aReason);
	if(FAILED(rp))
	{
		LogErrorVbox(onlyFile, "aguestprocess->waitforstart", rp);
		//retVal = ERRVBOX_WAITFORPROCESS_FAILED;
		goto cleanup;
	}

	rc = aGuestProcess->get_PID(&aPid);
	if(	FAILED (rc) )
	{
		retVal = ERRVBOX_GETPID_FAILED;
		//LogErrorVbox(onlyFile, "aguestprocess->get_pid", rc);
		goto cleanup;
	}

	printf("process created: %d \n", aPid);

	*uPid = aPid;

	rp = aGuestProcess->WaitFor(ProcessWaitForFlag_Terminate, 0, &aReason);
	if(FAILED(rp))
	{
		LogErrorVbox(onlyFile, "aguestprocess->waitforterminate", rp);
		retVal = ERRVBOX_WAITFORPROCESS_FAILED;
		goto cleanup;
	}

	printf("process terminate completed: %d \n", aPid);

cleanup:
	return retVal;
}

unsigned int CVBoxManager::PowerDown()
{
	unsigned int retVal = MVID_OK;
	IProgress	*aProgress;
	HRESULT rc;
	HRESULT rp;

	rc = iConsole->PowerDown(&aProgress);
	if (FAILED(rc))
	{
		retVal = ERRVBOX_POWERDOWN_FAILED;
		goto cleanup;
	}

	rp = aProgress->WaitForCompletion(-1);
	if (FAILED(rp))
	{
		retVal = ERRVBOX_WAITCOMPLETION_FAILED;
		goto cleanup;
	}

	rc = iSession->UnlockMachine();
	if (FAILED(rc))
	{
		retVal = ERRVBOX_UNLOCK_FAILED;
		goto cleanup;
	}

cleanup:

	SAFE_RELEASE(aProgress);
	return retVal;
}