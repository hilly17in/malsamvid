# include "stdafx.h"

DWORD
WINAPI
WaitForDetouredProcessExit(LPVOID lParam) 
{
	DWORD	status = SUCCESS;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	WCHAR	program[BUFSIZE];
    CHAR	szDllPath[1024];
	CHAR	pszDllPath[] = "ep1029.dll";
	PCHAR	pszFilePart = NULL;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	ZeroMemory(&gProcessInfo, sizeof(PROCESS_INFORMATION));
	
	ZeroMemory(program, BUFSIZE * sizeof(WCHAR));
	StringCbCopy(program, BUFSIZE* sizeof(WCHAR), (WCHAR *)lParam);
	
	DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

    if (!GetFullPathNameA(pszDllPath, ARRAYSIZE(szDllPath), szDllPath, &pszFilePart)) {
        printf("withdll.exe: Error: %s is not a valid path name..\n",
               pszDllPath);
        return 9002;
    }

	//printf("Program is %ws \n", program);	
	//printf("DLL Path is %s \n", szDllPath);

	// Start the child process. 
    if (!DetourCreateProcessWithDllEx(program, NULL,
                                      NULL, NULL, FALSE, dwFlags, NULL, NULL,
                                      &si, &pi, szDllPath, NULL)) {
	
		printf( "CreateProcess with program (to be analyzed) has failed (%d)\n", GetLastError() );
		ExitProcess(0);
	}

  //  if( !CreateProcess( NULL,   // No module name (use command line)
  //      program,				// Command line
  //      NULL,					// Process handle not inheritable
  //      NULL,					// Thread handle not inheritable
  //      FALSE,					// Set handle inheritance to FALSE
  //	  CREATE_SUSPENDED,       // Create in suspended mode and later resume
  //      NULL,					// Use parent's environment block
  //      NULL,					// Use parent's starting directory 
  //      &si,					// Pointer to STARTUPINFO structure
  //      &pi )					// Pointer to PROCESS_INFORMATION structure
  //  ) 
  //  {
  //      printf( "CreateProcess with program (to be analyzed) has failed (%d)\n", GetLastError() );
  //      ExitProcess(0);
  //  }

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
