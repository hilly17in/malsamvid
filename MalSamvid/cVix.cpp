#include "StdAfx.h"

// Not a member function. just a utility function.
void GetNameFromPath(const char *src, char *dst)
{
	const char *temp ;
	temp = strrchr(src, '\\');
	temp++;
	strcpy(dst, temp);
}

CVix::CVix(void)
{
	vmHandle = VIX_INVALID_HANDLE;
	hostHandle = VIX_INVALID_HANDLE;
	memset(FileName, '\0', MAX_NAME_INPATH);
}


CVix::~CVix(void)
{
	Vix_ReleaseHandle(vmHandle);
    VixHost_Disconnect(hostHandle);
}

void CVix::Connect()
{

	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;
	
	jobHandle = VixHost_Connect(VIX_API_VERSION,
                                CONNTYPE,
                                HOSTNAME,			// *hostName,
                                HOSTPORT,		// hostPort,
                                NULL,			// USERNAME
                                NULL,			// PASSWORD
                                0,				// options,
                                VIX_INVALID_HANDLE, // propertyListHandle,
                                NULL,				// *callbackProc,
                                NULL);				// *clientData);
	Sleep(2);
    err = VixJob_Wait(jobHandle, 
                      VIX_PROPERTY_JOB_RESULT_HANDLE, 
                      &hostHandle,
                      VIX_PROPERTY_NONE);
    if (VIX_FAILED(err)) {
		LogError(onlyFile, "VixHost_Connect", err);
		throw ex;
    }

    Vix_ReleaseHandle(jobHandle);

	return ;
}

void CVix::VMOpen(string vmxPath)
{

	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;
		
	jobHandle = VixVM_Open(hostHandle,
						   vmxPath.c_str(),
                           NULL,				// VixEventProc *callbackProc,
                           NULL);				// void *clientData);
	Sleep(2);
    err = VixJob_Wait(jobHandle, 
                      VIX_PROPERTY_JOB_RESULT_HANDLE, 
                      &vmHandle,
                      VIX_PROPERTY_NONE);
    if (VIX_FAILED(err)) {
		LogError(BothFilePrint, "VixVM_Open", err); //printf("VixVM_Open failed: %d. \n", err);
		throw ex;
    }

    Vix_ReleaseHandle(jobHandle);

	return;
}

void CVix::PowerVM()
{
	
	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;
	if(vmHandle == VIX_INVALID_HANDLE)
	{
		LogError(onlyFile, "PowerVM: VMHandle is Invalid", 0);
		return;
	}

	jobHandle = VixVM_PowerOn(vmHandle,
                              VMPOWEROPTIONS,
                              VIX_INVALID_HANDLE,
                              NULL,			// *callbackProc,
                              NULL);		// *clientData);
	Sleep(2);
    err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
    if (VIX_FAILED(err)) {
		LogError(BothFilePrint, "VixVM_PowerOn", err);
        throw ex;
    }

    Vix_ReleaseHandle(jobHandle);

	return;
}

void CVix::WaitForTools()
{

	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;

	// Wait until guest is completely booted.
	jobHandle = VixVM_WaitForToolsInGuest(vmHandle,
                                      300, // timeoutInSeconds
                                      NULL, // callbackProc
                                      NULL); // clientData

	Sleep(2);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		LogError(onlyFile, "VixVM_WaitForToolsInGuest", err);
		throw ex;
	}

	Vix_ReleaseHandle(jobHandle);
	return ;
}

void CVix::LogInGuest(string uname, string passwd)
{	
	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;

	
	// Authenticate for guest operations.
	jobHandle = VixVM_LoginInGuest(vmHandle,
								uname.c_str(), // userName
								passwd.c_str(), // password
								   0, // options
								   NULL, // callbackProc
								   NULL); // clientData
	Sleep(2);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
	   LogError(BothFilePrint, "VixVM_LoginInGuest", err);
	   throw ex;
	}

	Vix_ReleaseHandle(jobHandle);

}

void CVix::CopyFileToGuest(string filepath, string destDir)
{

	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;

	// prepare the dest path
			
	GetNameFromPath(filepath.c_str(), FileName);
	memset(dest_path, '\0', MAX_IMAGE_PATH);
	strcpy(dest_path, "C:\\Samples\\");				// I will copy the sample in pre-defined directory on virtual machine image
	strcat(dest_path, FileName);

	//
	// Copy the file.
	jobHandle = VixVM_CopyFileFromHostToGuest(vmHandle,
											filepath.c_str(),				// src name
											dest_path,						// dest name
											0,								// options
											VIX_INVALID_HANDLE,				// propertyListHandle
											NULL,							// callbackProc
											NULL);							// clientData
		
	Sleep(2);
	
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);

	if (VIX_OK != err) {
		LogError(BothFilePrint, "VixVM_CopyFileFromHostToGuest", err);
		throw ex;
	}

	Vix_ReleaseHandle(jobHandle);

	return ;
}

void CVix::RunPrograminGuest()
{

	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;

		// Run the target program.
		Bool jobCompleted;
		
		jobHandle = VixVM_RunProgramInGuest(vmHandle,
										"C:\\MalAnalyzer\\BIN\\u_malanalyze.exe",		// This is hardcoded ??
										dest_path,
										0, // options,
										VIX_INVALID_HANDLE, // propertyListHandle,
										NULL, // callbackProc,
										NULL); // clientData
		jobCompleted = FALSE;
		
		unsigned int is=0;

		cout << "Executing The Program in VM.. " << endl;
		while (!jobCompleted) {
		   err = VixJob_CheckCompletion(jobHandle, &jobCompleted);
		   if ( (VIX_OK != err) || (is == 3)){
			  break;
		   }
		   is++;
		   LogError(onlyFile, "VixVM_RunProgramInGuest", err);  //printf("\r VixVM_RunProgramInGuest %d , err = %0.4x", is, err);
		   Sleep(50000);
		}

		Vix_ReleaseHandle(jobHandle);
}

void CVix::CopyFileToHost(string outputDir)
{
	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;

	string strfilename = this->FileName;
	string destInHost;
	destInHost  = outputDir + "_" + strfilename ;

	cout << "Copying analysis files to " << destInHost << endl;

	jobHandle = VixVM_CopyFileFromGuestToHost(vmHandle,
											  "C:\\Samples\\analysis",				//"C:\\Samples\\analysis",	// src name in guest
											  destInHost.c_str(),					// dest name in host
											  0,									// options
											  VIX_INVALID_HANDLE,					// propertyListHandle
											  NULL,									// callbackProc
											  NULL);								// clientData
		Sleep(2);
		err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
		if (VIX_OK != err) {
			LogError(BothFilePrint, "VixVM_CopyFileFromGuestToHost", err);
			throw ex;
		}

		Vix_ReleaseHandle(jobHandle);

		return ;
}

void CVix::RevertToSnapshot(string snapName)
{
		
	
	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;
	VixHandle snapshotHandle;

	err = VixVM_GetNamedSnapshot(vmHandle, snapName.c_str(),
								 &snapshotHandle);

	if (VIX_OK != err) {
		  LogError(BothFilePrint, "VixVM_GetNamedSnapshot", err);
		  Vix_ReleaseHandle(snapshotHandle);
		  throw ex;
		}

		
	jobHandle = VixVM_RevertToSnapshot(vmHandle,
									   snapshotHandle,
									   0,					// options
									   VIX_INVALID_HANDLE,	// propertyListHandle
									   NULL,				// callbackProc
									   NULL);				// clientData
		Sleep(2);
		err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
		if (VIX_OK != err) {
		   LogError(BothFilePrint, "VixVM_RevertToSnapshot", err);
		   Vix_ReleaseHandle(snapshotHandle);
		  throw ex;
		}

		Vix_ReleaseHandle(jobHandle);
		Vix_ReleaseHandle(snapshotHandle);
}
	
void CVix::PowerOff()
{
	VixError err;
	exceptionVix ex;
	VixHandle jobHandle = VIX_INVALID_HANDLE;

	jobHandle = VixVM_PowerOff(vmHandle,
                               VIX_VMPOWEROP_NORMAL,
                               NULL, // *callbackProc,
                               NULL); // *clientData);
	Sleep(2);
    err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
    if (VIX_FAILED(err)) {
		LogError(BothFilePrint, "VixVM_PowerOff", err);
        throw ex;
    }

	Vix_ReleaseHandle(jobHandle);
}