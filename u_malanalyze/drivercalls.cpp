# include "stdafx.h"

# define USERMODE_MAFILTER_DEVICE L"\\\\.\\MAFilterSymLink"

DWORD
GetHandleToDriver()
{
	ghMADevice = CreateFile(USERMODE_MAFILTER_DEVICE, 
					 GENERIC_READ | GENERIC_WRITE,
					 FILE_SHARE_READ | FILE_SHARE_WRITE, 
					 0, 
					 OPEN_EXISTING,
					 FILE_ATTRIBUTE_NORMAL,
					 NULL) ;

	if (ghMADevice == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Initialize Driver: could not open handle to device \\\\.\\MalAnalyzeFilter \n") ;
		last_errno = GetLastError();
		return DRIVER_GETHANDLE_FAILED;
	}

	return SUCCESS ;
}


DWORD
InitializeDriver()
{
	DWORD status ;

	// start the driver.

	status = StartDriver();
	if (status != SUCCESS)
	{
		cout<<endl<<" * * *  Analyst Driver could not be started (error:" << GetLastError() << ")" <<endl;
		return status;
	}

	status = GetHandleToDriver();

	return status ;
}


DWORD 
SendPidToDriver(DWORD dwPid)
{
	DWORD dwBytesRet = 0;

	if(dwPid == 0)
		return ERROR_INVALID_PARAM;

	if(ghMADevice != INVALID_HANDLE_VALUE)
	{
		if(!DeviceIoControl(ghMADevice, 
					IOCTL_SEND_AND_INITIALIZE,
					(LPVOID)&dwPid,
					sizeof(DWORD),
					NULL,
					0,
					&dwBytesRet,
					NULL))
		{
			last_errno = GetLastError();
			wprintf(L"send pid to driver - deviceiocontrol has failed %d \n", last_errno);
			return ERROR_IOCTL_FAILURE;
		}

		return SUCCESS;
	} //if

	return ERROR_NULL_DEVICE ;

}

// this function runs through the loop of process id of malware and created/
// and terminates them one by one.
//
DWORD
MATerminateProcess(int processid)
{
	DWORD	dwBytesRet = 0;
	int		l_processid;
	DWORD	dwIoControlCode = IOCTL_TERMINATE_PROCESSID;
	vector<int>::iterator it = dataPid.begin();

	while(it != dataPid.end()) {
		l_processid = *it;
		
		if(ghMADevice)
		{
			if(!DeviceIoControl(ghMADevice, dwIoControlCode,
						(void *)&l_processid,
						sizeof(int),
						NULL,
						0,
						&dwBytesRet,
						NULL))
			{
				last_errno = GetLastError();
				wprintf(L"send terminate signal - deviceiocontrol has failed %d \n", last_errno);
				return ERROR_IOCTL_FAILURE;
			}

			wprintf(L"\n\n ++++ processes are being killed ++++ \n\n");
			return SUCCESS;
		} //if

		it++;
	}


	return ERROR_NULL_DEVICE ;

}

int
SendServicePid(int processid)
{
	DWORD dwBytesRet;

	if(ghMADevice != INVALID_HANDLE_VALUE)
	{
		if(!DeviceIoControl(ghMADevice, 
					IOCTL_SEND_SERVICE_PID,
					(LPVOID)&processid,
					sizeof(DWORD),
					NULL,
					0,
					&dwBytesRet,
					NULL))
		{
			last_errno = GetLastError();
			wprintf(L"send service pid - deviceiocontrol has failed %d \n", last_errno);
			return ERROR_IOCTL_FAILURE;
		}

		return SUCCESS;
	} //if

	return ERROR_NULL_DEVICE ;

}

DWORD
StartDriver()
{
	SC_HANDLE hSCM;
	SC_HANDLE hService;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCM == NULL)
		return ERROR_FAILURE;
	
	hService = OpenService(hSCM, L"MalProber", SERVICE_ALL_ACCESS);
	if (hService == NULL)
		return ERROR_FAILURE;

	if( !StartService(hService, 0, NULL))
		if(ERROR_SERVICE_ALREADY_RUNNING != GetLastError())	
			return ERROR_FAILURE;
	
	cout<<endl<<" * * *  MalProber Analyst Driver started successfully * * * "<<endl;

	return SUCCESS;

}

DWORD StopDriver()
{
	SC_HANDLE hSCM;
	SC_HANDLE hService;
	SERVICE_STATUS ss;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCM == NULL)
		return ERROR_FAILURE;
	
	hService = OpenService(hSCM, L"MalProber", SERVICE_ALL_ACCESS);
	if (hService == NULL)
		return ERROR_FAILURE;

	
	if( !ControlService(hService, SERVICE_CONTROL_STOP, &ss))
		return ERROR_FAILURE;

	cout<<".";

	return SUCCESS;

}