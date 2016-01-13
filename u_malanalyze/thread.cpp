# include "stdafx.h"

PVOID 
GetPebAddress(HANDLE ProcessHandle)
{
    _NtQueryInformationProcess NtQueryInformationProcess =
        (_NtQueryInformationProcess)GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
    PROCESS_BASIC_INFORMATION pbi;

    NtQueryInformationProcess(ProcessHandle, 0, &pbi, sizeof(pbi), NULL);

    return pbi.PebBaseAddress;
}


void
WriteCommandLineinReport(unsigned long pid)
{
	HANDLE		processHandle;
	PVOID		pebAddress;
    PVOID		rtlUserProcParamsAddress;
    UNICODE_STRING commandLine;
    WCHAR		*commandLineContents;
	WCHAR		tmp[1024];
	WCHAR		ImageFileName[MAX_PATH];
	DWORD		dwBytesRead;

	if ((processHandle = OpenProcess(
        PROCESS_QUERY_INFORMATION | /* required for NtQueryInformationProcess */
        PROCESS_VM_READ, /* required for ReadProcessMemory */
        FALSE, pid)) == 0)
    {
		cout<<"could not open process"<<endl;
		FeatureVector[16].value = 3;  
		FeatureVector[16].count++;//gSusp = gSusp + 3;
        goto hidden ;
    }

	FeatureVector[16].value = 2;  
	FeatureVector[16].count++;	//gSusp = gSusp + 3;

	GetProcessImageFileName(processHandle, ImageFileName, MAX_PATH);
	StringCbPrintf(tmp, 1024 * sizeof(WCHAR), 
				   L"creates a process (%d): %s", 
				   pid, ImageFileName);
   
	pebAddress = GetPebAddress(processHandle);

    /* get the address of ProcessParameters */
    if (!ReadProcessMemory(processHandle, (PCHAR)pebAddress + 0x10,
        &rtlUserProcParamsAddress, sizeof(PVOID), NULL))
    {
		//cout<<"get peb addr failed"<<endl;
		goto cleanup;	
	}

    /* read the CommandLine UNICODE_STRING structure */
    if (!ReadProcessMemory(processHandle, (PCHAR)rtlUserProcParamsAddress + 0x40,
        &commandLine, sizeof(commandLine), &dwBytesRead))
    {
		//cout<<"read process memory: ( "<<GetLastError()<<" )"<<endl;
		goto cleanup;	
	}

	if(dwBytesRead == 0)
		goto cleanup;

    /* allocate memory to hold the command line */
    commandLineContents = (WCHAR *)malloc(commandLine.Length + 1);

    /* read the command line */
    if (!ReadProcessMemory(processHandle, commandLine.Buffer,
        commandLineContents, commandLine.Length, NULL))
    {
		//cout<<"read process memory for commadline ( "<<GetLastError()<<" )"<<endl;
		goto cleanup;
    }

	StringCbPrintf(tmp, 1024 * sizeof(WCHAR), 
				   L"creates a process (%d): (%s) with options (%s)", 
				   pid, ImageFileName, commandLineContents);
	
	free (commandLineContents);

	goto cleanup;

hidden:
	StringCbPrintf(tmp, MAX_PROGRAM_LEN * sizeof(WCHAR),
				   L"created either a short lived or hidden process: pid = %d", 
				   pid);
	

cleanup:

	if(tmp != NULL)
	{
		dproc.replace(tmp);
		dataProcess.push_back(dproc);
	}
	
	CloseHandle(processHandle);

	return ;
}



DWORD
WINAPI
CommunicateWithDriver(LPVOID lParam)
{
	DWORD status = SUCCESS ;
	HRESULT		hResult ;	
	HANDLE		hPort ;
	RecvdBuffer	MessageBuffer;
	DWORD	context =1;

	UNREFERENCED_PARAMETER(lParam) ;

	hResult = FilterConnectCommunicationPort(MAFILTER_PORT_NAME, 0, 
											&context, sizeof(DWORD), 
											NULL, 
											&hPort) ;
	if (hResult != S_OK)
	{
		return GetLastError();
	}

	while (1)
	{
		hResult = FilterGetMessage(hPort, &MessageBuffer.header, 
			sizeof(RecvdBuffer), NULL) ;

		if (hResult != S_OK)
		{
			continue;
		}

		switch (MessageBuffer.buffer.Type)
		{
		case NotifyProcess:
			
				if (MessageBuffer.buffer.NPData.Create)
				{
					printf("\nmonitoring: process (pid:%d) created this process (pid:%d)\n", MessageBuffer.buffer.NPData.ParentId, MessageBuffer.buffer.NPData.Pid);
					dataPid.push_back(MessageBuffer.buffer.NPData.Pid);
					WriteCommandLineinReport(MessageBuffer.buffer.NPData.Pid);
				}
				else
				{
					printf("\nmonitoring: process [(pid: %d):Parent Pid: %d] has exited.\n", MessageBuffer.buffer.NPData.Pid, MessageBuffer.buffer.NPData.ParentId);
					dataPid.erase(std::remove(dataPid.begin(), dataPid.end(), MessageBuffer.buffer.NPData.Pid), dataPid.end());
				}

				break;
			
		case NotifyBehavior:
			switch(MessageBuffer.buffer.NBData.uOperationIndex)
			{
			case CreateNewFile:

				//dc.replace(MessageBuffer.buffer.NBData.FilePath);
				//dataCreate.push_back(dc);
	
				if(NULL==wcsstr(MessageBuffer.buffer.NBData.FilePath, L"\\MalAnalyzer\\BIN\\ignore.txt"))
				{
					hashinfo t;
					t.SetFilePath(MessageBuffer.buffer.NBData.FilePath);
					dataCreate.push_back(t);
				}

				break;

			case OpenExistFile:

				dop.replace(MessageBuffer.buffer.NBData.FilePath);
				dataOpen.push_back(dop);

				break;

			case ReadingFile:

				dr.replace(MessageBuffer.buffer.NBData.FilePath);
				dataRead.push_back(dr);
				
				break;

			case WritingFile:

				if(NULL==wcsstr(MessageBuffer.buffer.NBData.FilePath, L"\\BIN\\ignore.txt"))
				{
					dw.replace(MessageBuffer.buffer.NBData.FilePath);
					dataWrite.push_back(dw);
				}

				break;
				
			case SendsIoctl:

				di.replace(MessageBuffer.buffer.NBData.FilePath);
				dataIoctl.push_back(di);

				break;

			case ChangesPerm:
					
				ds.replace(MessageBuffer.buffer.NBData.FilePath);
				dataSetSec.push_back(ds);

				break;

			case DeletesFile:
				
				dd.replace(MessageBuffer.buffer.NBData.FilePath);
				dataDel.push_back(dd);

				break;

			case SetTimeStamp:

				dt.replace(MessageBuffer.buffer.NBData.FilePath);
				datatimestamp.push_back(dt);
	
				break;
				
			} // internal switch for operation index	
			break;
		} //switch
	} // while

	return status ;
}


DWORD
WINAPI
CommunicateWithDriver2(LPVOID lParam)
{
	DWORD status = SUCCESS ;
	HRESULT		hResult ;	
	HANDLE		hPort ;
	RegBuffer	MessageBuffer;
	DWORD context = 2 ;

	UNREFERENCED_PARAMETER(lParam) ;

	hResult = FilterConnectCommunicationPort(MAFILTER_PORT_NAME, 0, 
											&context, sizeof(DWORD), 
											NULL, 
											&hPort) ;
	if (hResult != S_OK)
	{
		return GetLastError();
	}

	while (1)
	{
		hResult = FilterGetMessage(hPort, &MessageBuffer.header, 
			sizeof(RegBuffer), NULL) ;

		if (hResult != S_OK)
		{
			continue;
		}

		switch (MessageBuffer.RegEvent.OpIndex)
		{
		case CreateRegKey:
	
			if (MessageBuffer.RegEvent.wRegPath) {

				dcrk.replace(MessageBuffer.RegEvent.wRegPath);	
				dataCreateRegKey.push_back(dcrk);
			}

			break;

		case SetValueKey:
			{
				WCHAR final[1300];
				ZeroMemory(final, sizeof(final));

				if(MessageBuffer.RegEvent.wRegPath)
					StringCbCat(final, 1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wRegPath);
					
					//wcscat(final, MessageBuffer.RegEvent.wRegPath);

				StringCbCat(final, sizeof(WCHAR), L"  ");

				if(MessageBuffer.RegEvent.wValueName)
					StringCbCat(final,  1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wValueName);

				if(final) {
					dsek.replace(final);
					dataSetValueKey.push_back(dsek);
				}
				
			}

			break;

		case DeleteKey:
			{
				WCHAR final[1300];
				ZeroMemory(final, sizeof(final));

				if (MessageBuffer.RegEvent.wRegPath) 
					StringCbCat(final,  1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wRegPath);

				StringCbCat(final, sizeof(WCHAR), L"   ");

				if(MessageBuffer.RegEvent.wValueName)
					StringCbCat(final,  1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wValueName);

				if(final) {				
					delk.replace(final);	
					dataDeleteKey.push_back(delk);
				}
			}
			break;

		case OpenRegKey:
			{
				WCHAR final[1300];
				ZeroMemory(final, sizeof(final));

				if(MessageBuffer.RegEvent.wRegPath)
				{
					StringCbCat(final,  1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wRegPath);

					if(final) {				
						openk.replace(final);	
						dataOpenKey.push_back(openk);
					}
				}

			}
			break;

		case EnumRegKey:
			{
				WCHAR final[1300];
				ZeroMemory(final, sizeof(final));

				if(MessageBuffer.RegEvent.wRegPath)
				{
					StringCbCat(final,  1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wRegPath);

					if(final) {				
						enumk.replace(final);	
						dataEnumKey.push_back(enumk);
					}
				}

			}

			break;

		case RegEnumValueKey:
			{
				WCHAR final[1300];
				ZeroMemory(final, sizeof(final));

				if(MessageBuffer.RegEvent.wRegPath)
				{
					StringCbCat(final,  1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wRegPath);

					if(final) {				
						enumv.replace(final);	
						dataEnumValueKey.push_back(enumv);
					}
				}

			}

			break;

		case RegQueryKey:
			{
				WCHAR final[1300];
				ZeroMemory(final, sizeof(final));

				if(MessageBuffer.RegEvent.wRegPath)
				{
					StringCbCat(final,  1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wRegPath);

					if(final) {				
						queryk.replace(final);	
						dataQueryKey.push_back(queryk);
					}
				}

			}

			break;

		case RegQueryValueKey:
			{
				WCHAR final[1300];
				ZeroMemory(final, sizeof(final));

				if(MessageBuffer.RegEvent.wRegPath)
				{
					StringCbCat(final,  1300 * sizeof(WCHAR), MessageBuffer.RegEvent.wRegPath);

					if(final) {				
						queryv.replace(final);	
						dataQueryValueKey.push_back(queryv);
					}
				}

			}

			break;


		} //switch
	} // while

	return status ;
}



DWORD
WINAPI
WaitForProcessExit(LPVOID lParam) 
{
	DWORD	status = SUCCESS;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	WCHAR	program[BUFSIZE];

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	ZeroMemory(&gProcessInfo, sizeof(PROCESS_INFORMATION));
	
	ZeroMemory(program, BUFSIZE * sizeof(WCHAR));
	StringCbCopy(program, BUFSIZE* sizeof(WCHAR), (WCHAR *)lParam);
	
	// Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line)
        program,				// Command line
        NULL,					// Process handle not inheritable
        NULL,					// Thread handle not inheritable
        FALSE,					// Set handle inheritance to FALSE
		CREATE_SUSPENDED,       // Create in suspended mode and later resume
        NULL,					// Use parent's environment block
        NULL,					// Use parent's starting directory 
        &si,					// Pointer to STARTUPINFO structure
        &pi )					// Pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf( "CreateProcess with program (to be analyzed) has failed (%d)\n", GetLastError() );
        ExitProcess(0);
    }

	CopyMemory(&gProcessInfo, &pi, sizeof(PROCESS_INFORMATION));
	dataPid.push_back(pi.dwProcessId);
	
	status = SendPidToDriver(pi.dwProcessId);
	if (status != SUCCESS)
	{
		wprintf(L"could not send the pid to driver - will stop the analysis and program. (error: %d)\n", status);
		TerminateProcess(pi.hProcess, 0);
		ExitProcess(0);
	}

	printf("\n++++   process to be analyzed is %d   ++++\n", pi.dwProcessId);

	Sleep(0);
	ResumeThread(pi.hThread);

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	return 0;
}

int GetServicePid()
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	wstring serstr = L"\\windows\\system32\\services.exe";
	wstring temp;
	int pos = -1;


	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return -1;

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);
	
	for ( i = 0; i < cProcesses; i++ ) {
		if( aProcesses[i] != 0 ) {
         
			HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, aProcesses[i] );

			// Get the process name.

			if (NULL != hProcess )
			{
				HMODULE hMod;
				DWORD cbNeeded;

				if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
					 &cbNeeded) )
				{
					GetModuleBaseName( hProcess, hMod, szProcessName, 
									   sizeof(szProcessName)/sizeof(TCHAR) );

					temp = szProcessName;
					pos = temp.find(serstr);

					if(pos > 0 ) 
						return aProcesses[i] ;
					else 
						continue;
				}
			}


		} //if
	} //for
	return -1;
} // getservicepid

