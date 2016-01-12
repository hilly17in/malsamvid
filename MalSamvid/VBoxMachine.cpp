#include "StdAfx.h"

int CVBoxMachine::count = 0;
CVBoxManager CVBoxMachine::vbmManager;

CVBoxMachine::CVBoxMachine(void)
{
	if(count == 0)
	{
		vbmManager.Init();
	}

	++count;
	
}

CVBoxMachine::~CVBoxMachine(void)
{
	--count;
	
	if (count == 0)
	{
		vbmManager.Release();
	}
}

unsigned int CVBoxMachine::SetVM(wchar_t *szVMName)
{
	unsigned int retVal = MVID_OK;
	
	bsMachineName = SysAllocString(szVMName);	// the string has to be replaced.

	retVal = vbmManager.FindMachine(bsMachineName);
	if (retVal != MVID_OK)
		return retVal;

	return retVal;
}

unsigned int CVBoxMachine::SetVM(const char *szVMName)
{
	unsigned int retVal = MVID_OK;
	
	bsMachineName = WSysAllocString(szVMName);	// the string has to be replaced.

	retVal = vbmManager.FindMachine(bsMachineName);
	if (retVal != MVID_OK){
		printf("Error in findmachine : %d \n", retVal);
		return retVal;
	}

	return retVal;
}

unsigned int CVBoxMachine::powerOn()
{
	unsigned int retVal = MVID_OK;

	retVal = vbmManager.LaunchVM();
	if(retVal != MVID_OK)
		printf("Error in LaunchVM %d \n", retVal);

	return retVal;
}

unsigned int CVBoxMachine::logon(wchar_t *userName, wchar_t *password)
{
	unsigned int retVal = MVID_OK;

	bsUserName = SysAllocString(userName);
	bsPassword = SysAllocString(password);

	retVal = vbmManager.CreateGuestSession(bsUserName, bsPassword);

	return retVal;
}

unsigned int CVBoxMachine::logon(const char *userName, const char *password)
{
	unsigned int retVal = MVID_OK;

	bsUserName = WSysAllocString(userName);
	bsPassword = WSysAllocString(password);

	retVal = vbmManager.CreateGuestSession(bsUserName, bsPassword);

	return retVal;
}

unsigned int CVBoxMachine::IsSystemReady(char *dirPath)
{
	BSTR bsDirPath;
	BOOL dirExists;

	bsDirPath = WSysAllocString(dirPath);

	vbmManager.DirectoryExists(bsDirPath, &dirExists);
	if(dirExists == TRUE)
		return MVID_OK;

	return ERRVBOX_SYSTEM_NOTREADY;
}

unsigned int CVBoxMachine::CopyFileToVM(const char *src)
{
	BSTR bSrc;
	

	if( (src == NULL) || (strlen(src) < 4) )
		return ERRVBOX_INVALID_ARGS;


	// prepare the dest path
	GetNameFromPath(src, FileName);
	memset(dest_path, '\0', MAX_IMAGE_PATH);
	strcpy(dest_path, "C:\\Samples\\");				// I will copy the sample in pre-defined directory on virtual machine image
	strcat(dest_path, FileName);

	bSrc = WSysAllocString(src);
	bDst = WSysAllocString(dest_path);

	return vbmManager.CopyFileToVM(bSrc, bDst);
}

unsigned int CVBoxMachine::CopyFileFromVM(string outputDir)
{
	BSTR bSrc;
	BSTR bDst;

	if( outputDir.length() < 4 )
		return ERRVBOX_INVALID_ARGS;

	string strfilename = this->FileName;
	string destInHost;
	destInHost  = outputDir + "_" + strfilename ;

	wchar_t srcOnGuest[] = L"C:\\Samples\\analysis";
	bSrc = SysAllocString(srcOnGuest);
	bDst = WSysAllocString(destInHost.c_str());

	return vbmManager.CopyFileFromVM(bSrc, bDst);

}

unsigned int CVBoxMachine::RunProgram(ULONG *argPid)
{
	BSTR	bProgPath;
	BSTR	bAnalyzerProg;
	//ULONG	uPid;
	unsigned int retVal = MVID_OK;

	//wchar_t szAnalyzer[] = L"C:\\MalAnalyzer\\BIN\\u_malanalyze.exe ";
	char szAnalyzer[] = "C:\\MalAnalyzer\\BIN\\u_malanalyze.exe ";
	strcat(szAnalyzer," ");
	strcat(szAnalyzer, dest_path);

	//bAnalyzerProg = SysAllocString(szAnalyzer);
	bAnalyzerProg = WSysAllocString(szAnalyzer);

	bProgPath = WSysAllocString(dest_path);

	//retVal = vbmManager.RunProgram(bAnalyzerProg, bProgPath, argPid);
	retVal = vbmManager.RunProgram(bAnalyzerProg, NULL, argPid);
	
	return retVal;
}

unsigned int CVBoxMachine::powerOff()
{
	return vbmManager.PowerDown();
}