// u_malanalyze.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD	status = SUCCESS;
	wchar_t program_name[BUFSIZE];
	HANDLE  hComThread, hComThread2, hNwThread, hPacketThread ; 
	DWORD	dwThreadId, dwThreadId2, dwNwThreadId, dwPacketThreadId ;	

	printf("\n\n ***** \n\n MalSamvid Malware Analysis Tool \n\n *** \n\n");
	
	// check for invalid usage of the malware anlayzer
	if (argc < 2)
	{
		wprintf(L"usage: u_malanalyze.exe <sample malware.exe> \n second argument is the program name to be analyzed \n");
		return 0;
	}
	
	// check if supplied length is more than 255 characters.
	if (wcslen(argv[1]) >= MAX_PROGRAM_LEN)
	{
		wprintf(L"usage: please note second argument can not exceed 255 characters\n");
		return 0;
	}

	memset(program_name, 0, BUFSIZE * sizeof(WCHAR));
	int xz;
	for (xz = 0; xz < 17; xz++)
	{
		FeatureVector[xz].count = 0;
		FeatureVector[xz].value = 0;
	}

	// passed all the tests - copy the program name and calculate hash
	StringCbCopy(program_name, MAX_PROGRAM_LEN * sizeof(WCHAR), argv[1]);
	prog = program_name;
	extractfinalname((wchar_t *)prog.c_str(), malwarename);

	h_prog.SetFilePath(prog.c_str());
	h_prog.CalcAllHashes();
	h_prog.GetMD5(TempMD5);

	// create the program command line
	if(argc>2){
		int cmdlinearg=2;
		StringCbCat(program_name, MAX_PROGRAM_LEN * sizeof(WCHAR), L" ");
		
		while(cmdlinearg<argc)
			StringCbCat(program_name, BUFSIZE * sizeof(WCHAR), argv[cmdlinearg++]);
	}

	SetupDirectory();

	if(SUCCESS == SetupPacketCapture())
	{
			hPacketThread = CreateThread(NULL, 0,
										 CapturePackets,
										 NULL,
										 0,
										 &dwPacketThreadId);
			if (hPacketThread == NULL)
			{
				last_errno = GetLastError();
				wprintf(L"could not create packet capture thread %d\n", last_errno);
			}
	}
	
	fflush(stdin);

	// check print - to remove
	wcout<<endl<<L"++++ Program to be analyzed "<<prog<<L"  ++++ "<<endl;

	// initialize the driver - get handle etc.
	status = InitializeDriver();
	if (status != SUCCESS)
	{
		wprintf(L"driver could not be initialized %x\n", status);
		return 0;
	}

	// TO DO
	// take a memory snapshot
	// here.

	//
	// now create a thread which will look for network ports and connections basically
	//
	hNwThread = CreateThread(NULL,
				 0,
				 NetworkPortMonitorThread,
				 NULL,
				 0,
				 &dwNwThreadId);
	if (hNwThread == NULL)
	{
		last_errno = GetLastError();
		wprintf(L"could not create network monitor thread %d\n", last_errno);
		goto cleanup;
	}

	// TODO (ndis driver will allow to identify the packet based on pid and pcap will not)
	// now create a thread to capture the packets transferred in this duration
	//

	// 
	// now create a thread which will communicate to filter driver - recv messages and write to file
	//
	hComThread = CreateThread(NULL,
				 0,
				 CommunicateWithDriver,
				 NULL,
				 0,
				 &dwThreadId);
	if (hComThread == NULL)
	{
		last_errno = GetLastError();
		wprintf(L"could not create comm thread %d\n", last_errno);
		goto cleanup;
	}

	// 
	// now create a thread which will communicate to filter driver - recv registry messages
	//
	hComThread2 = CreateThread(NULL,
				 0,
				 CommunicateWithDriver2,
				 NULL,
				 0,
				 &dwThreadId2);
	if (hComThread2 == NULL)
	{
		last_errno = GetLastError();
		wprintf(L"could not create comm thread %d\n", last_errno);
		goto cleanup;
	}


	int sPid;
	sPid = GetServicePid();
	if(sPid != -1)
		SendServicePid(sPid);

	// what if process exits by own?
	// wait for its exit
	// create a thread

	HANDLE	hWaitThread ;
	hWaitThread = CreateThread(NULL, 0,
				 WaitForDetouredProcessExit,
				 program_name,
				 0,
				 &dwThreadId);
	if (hWaitThread == NULL)
	{
		last_errno = GetLastError();
		wprintf(L"could not create wait thread %d\n", last_errno);
		goto cleanup;
	}

	//HANDLE	hWaitThread ;
	//hWaitThread = CreateThread(NULL, 0,
	//			 WaitForProcessExit,
	//			 program_name,
	//			 0,
	//			 &dwThreadId);
	//if (hWaitThread == NULL)
	//{
	//	last_errno = GetLastError();
	//	wprintf(L"could not create wait thread %d\n", last_errno);
	//	goto cleanup;
	//}

	// wait for user input
	wprintf(L"press enter to stop the analysis..\n");
	getchar();
	
	// and kill the process
	status = MATerminateProcess(gProcessInfo.dwProcessId);
	if(status != SUCCESS)
	{
		wprintf(L"could not kill processes. \n");
	}


	cout<<"I am now analyzing the results and generating the report. "<< endl ; 
	StopDriver();

	// Stop the packet capturing thread
	TerminateThread(hPacketThread, 0);

	// now display the report
	DisplayReport();
	

cleanup:

	if (ghMADevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(ghMADevice);
	}

	if (hComThread != NULL)
	{
		CloseHandle(hComThread);
	}

	if (hComThread2 != NULL)
	{
		CloseHandle(hComThread2);
	}

	if (hWaitThread != NULL)
	{
		CloseHandle(hWaitThread);
	}
	
	if (hNwThread != NULL)
	{
		CloseHandle(hNwThread);
	}

	if(gProcessInfo.hProcess)
	{
		CloseHandle(gProcessInfo.hProcess);
	}

	return 0;
}

