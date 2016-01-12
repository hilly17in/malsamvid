#pragma once

BSTR WSysAllocString(const char *);

unsigned int GetLastErrorInfo(char **);

class CVBoxManager
{
private:
		IVirtualBox		*iVirtualBox;
		ISession		*iSession;
		IConsole		*iConsole;
		IGuest			*iGuest;
		IGuestSession	*iGuestSession;
		IMachine		*iMachine;		
		BSTR			guid;		

		unsigned int GetSession();
		unsigned int GetGuest();
		void SetExtendedLastCOMError(HRESULT);

public:
	unsigned int Init();
	void Release();

	unsigned int FindMachine(BSTR bsMachineName);
	unsigned int LaunchVM();
	unsigned int CreateGuestSession(BSTR aUser, BSTR aPass);
	unsigned int GetExtendedLastCOMError();
	unsigned int DirectoryExists(BSTR, BOOL *);
	unsigned int CopyFileToVM(BSTR, BSTR);
	unsigned int CopyFileFromVM(BSTR, BSTR);
	unsigned int RunProgram(BSTR, BSTR, ULONG *);
	unsigned int PowerDown();

	CVBoxManager(void);
	~CVBoxManager(void);
};
