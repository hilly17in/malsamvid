#pragma once


#define  CONNTYPE    VIX_SERVICEPROVIDER_VMWARE_WORKSTATION

#define  HOSTNAME ""
#define  HOSTPORT 0
#define  USERNAME "h0m3"
#define  PASSWORD "admin123"

#define  VMPOWEROPTIONS   VIX_VMPOWEROP_NORMAL			//VIX_VMPOWEROP_LAUNCH_GUI   // Launches the VMware Workstaion UI
														// when powering on the virtual machine.


class exceptionVix
{
//public:
//	exceptionVix(){}
//	~exceptionVix(){}
};

class CVix
{
private:

	VixHandle vmHandle ;
	VixHandle hostHandle ;

	char dest_path[MAX_IMAGE_PATH];		
	char FileName[MAX_NAME_INPATH] ;

public:
	CVix(void);
	~CVix(void);

	

	void Connect();
	void VMOpen(string vmxPath);
	void PowerVM();
	void WaitForTools();
	void LogInGuest(string uname, string passwd);
	void CopyFileToGuest(string filepath, string destDir);
	void RunPrograminGuest();
	void CopyFileToHost(string outputDir);
	void RevertToSnapshot(string snapName);
	void PowerOff();
};


void GetNameFromPath(const char *src, char *dst);