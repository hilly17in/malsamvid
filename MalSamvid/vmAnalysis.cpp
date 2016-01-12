#include "stdafx.h"

sVMAnalyzer gDetail;

DWORD WINAPI VMWareThread( LPVOID lpParam )
{
	CVix objVix;
	try{
		objVix.Connect();
		objVix.VMOpen(gDetail.m_VMPath);
		objVix.PowerVM();
		objVix.WaitForTools();
		objVix.LogInGuest(gDetail.m_uname, gDetail.m_password);
		objVix.CopyFileToGuest(gDetail.m_FilePath, gDetail.m_WinDir);
		objVix.RunPrograminGuest();
		objVix.CopyFileToHost(csettings.m_outputDir);
		objVix.RevertToSnapshot(gDetail.m_Snapshot);
		objVix.PowerOff();
	}
	catch(exceptionVix e)
	{
		UNREFERENCED_PARAMETER(e);
		cout << "error in VIX has occurred." << endl;
		ReleaseMutex(ghMutexVMWare);
		return 0;
	}

	Sleep(1000);
	ReleaseMutex(ghMutexVMWare);

	return 0;


} 

unsigned int VMWareAnalysis(
  sVMAnalyzer &sDetails
)
{

	gDetail = sDetails;
	DWORD threadID;
	string mesg = "Analyzing" + gDetail.m_FilePath + "with VMWare";

	LogError(BothFilePrint, mesg, 0);
	hThreadArray[0] = CreateThread(NULL, 0,
							VMWareThread,
							NULL,
							0,
							&threadID);

	if(hThreadArray[0] == NULL)
		LogError(onlyFile, "CreateThread VMWareThread", GetLastError());

	Sleep(100);
	return 0;
}