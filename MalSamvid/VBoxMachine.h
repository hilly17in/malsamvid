#pragma once

class CVBoxMachine
{
private:

	static CVBoxManager vbmManager;
	static int count ;

	BSTR	bsMachineName;
	BSTR	bsUserName;
	BSTR	bsPassword;

	char dest_path[MAX_IMAGE_PATH];		
	BSTR bDst;
	char FileName[MAX_NAME_INPATH] ;

public:
	CVBoxMachine(void);
	~CVBoxMachine(void);


	unsigned int SetVM(wchar_t *szVMName);
	unsigned int SetVM(const char *szVMName);

	unsigned int powerOn();
	
	unsigned int logon(wchar_t *userName, wchar_t *password);
	unsigned int logon(const char *userName, const char *password);
	
	unsigned int IsSystemReady(char *);
	unsigned int CopyFileToVM(const char *);
	unsigned int CopyFileFromVM(string outputDir);
	
	unsigned int RunProgram(ULONG *);
	unsigned int powerOff();

};
