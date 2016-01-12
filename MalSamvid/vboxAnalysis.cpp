#include "stdafx.h"

sVMAnalyzer gDetail_vbox;

DWORD WINAPI VBoxThread( LPVOID lpParam )
{
	CVBoxMachine myXP;
	unsigned int retVal = MVID_OK;
	
	retVal = myXP.SetVM(gDetail_vbox.m_VMPath.c_str());
	if(MVID_OK != retVal)
	{
		LogError(BothFilePrint, "VboxMachine::SetVM", retVal);
		goto cleanup;
	}

	printf("after setVM \n");
	retVal = myXP.powerOn();
	if(MVID_OK != retVal)
	{
		LogError(BothFilePrint, "VboxMachine::PowerOn", retVal);
		goto cleanup;
	}
	printf("after poweron \n");

	retVal = myXP.logon(gDetail_vbox.m_uname.c_str(), gDetail_vbox.m_password.c_str());
	if(MVID_OK != retVal)
	{
		LogError(BothFilePrint, "VboxMachine::LogOn", retVal);
		goto poweroff;
		
	}
	printf("after logon \n");
	retVal =  myXP.IsSystemReady("C:\\Windows\\");
	if(MVID_OK != retVal)
	{
		LogError(BothFilePrint, "VboxMachine::IsSystemReady", retVal);
		goto poweroff;
	}
	else
	{
		printf("system is ready \n");
	}

	ULONG	uProcessId;

	retVal = myXP.CopyFileToVM(gDetail_vbox.m_FilePath.c_str());
	if ( MVID_OK != retVal )
	{
		LogError(onlyFile, "VboxMachine::CopyFileToVM", retVal);
		goto poweroff;
	}

	printf("after copyfiletovm: press enter to proceed\n");
	Sleep(1000);
	getchar();
	printf("ok.. will now proceed\n");

	retVal = myXP.RunProgram(&uProcessId);
	if ( MVID_OK != retVal )
	{
		LogError(onlyFile, "VboxMachine::RunProgram", retVal);
		goto poweroff;
	}

	printf("after runprogram\n");
	getchar();
	Sleep(5000);
	
	retVal = myXP.CopyFileFromVM(csettings.m_outputDir);
	if ( MVID_OK != retVal )
	{
		LogError(onlyFile, "VboxMachine::CopyFileFromVM", retVal);
		goto poweroff;
	}

	Sleep(3000);
	printf("after copyfilefromvm\n");

	poweroff:
	retVal = myXP.powerOff();
	if ( MVID_OK != retVal )
	{
		LogError(onlyFile, "VboxMachine::powerOff", retVal);
		goto cleanup;
	}

	cleanup:
	// release the mutex
	ReleaseMutex(ghMutexVbox);

	return 0;
} 

unsigned int VBoxAnalysis(
  sVMAnalyzer &sDetails
)
{

	gDetail_vbox = sDetails;

	string mesg = "Analyzing " + gDetail_vbox.m_FilePath + " with VirtualBox";
	LogError(BothFilePrint, mesg, 0);

	DWORD	threadID;

	hThreadArray[1] = CreateThread(NULL, 0,
							VBoxThread,
							NULL,
							0,
							&threadID);

	if(hThreadArray[1] == NULL)
		LogError(onlyFile, "CreateThread VBoxThread", GetLastError());
	else
		Sleep(100);

	return 0;
}