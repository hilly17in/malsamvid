#include "StdAfx.h"

CLog gLog;

CLog::CLog(void)
{
	char szPath[MAX_IMAGE_PATH];
	GetModuleFileName(NULL, szPath, MAX_IMAGE_PATH);
	
	string logfilename = szPath;
	logfilename.append("_log.txt");

	logfile.open(logfilename.c_str(), ios::app);

}


CLog::~CLog(void)
{
	logfile << " -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	logfile.close();
}

void CLog::LogMessage(string funcName, const char *mesg)
{
	// get time 
	time_t rawtime;
	struct tm * timeinfo;
	char strtime[80];
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (strtime, 80, "%I:%M%p.",timeinfo);

	if(logfile.is_open())
		logfile << strtime << ": " << "Function: " << funcName << ": " << mesg << endl;
}

void LogError(eLogMode mode, string funcName, unsigned int iError)
{
	unsigned int vixFlag = 0;
	const char *Message;

	if( !funcName.compare(0, 6, "VixVM_") ) { // this is a VIX API Error.
		Message = Vix_GetErrorText(iError, NULL);
	}
	else{
		char t[80];
		if(iError != 0)
			sprintf(t, "%d: Internal Error", iError);//Message = Mal_GetErrorText();
		else
			sprintf(t, "success");

		Message = t;
	}
	switch(mode)
	{
		case BothFilePrint:

		case onlyFile:

		gLog.LogMessage(funcName, Message);

		if(mode == onlyFile)
			break;

	case onlyPrint:

		cout << funcName << ":" << Message << endl;

		break;


	}
}


void LogErrorVbox(eLogMode mode, string funcName, HRESULT rc)
{

	const char *Message;
	HRESULT rce;
	BSTR errorDescription = NULL;
	IErrorInfo *errorInfo;
	rce = GetErrorInfo(0, &errorInfo);
	rce = errorInfo->GetDescription(&errorDescription);

	char t[256];
	sprintf(t, "%s: %S\n", funcName.c_str(), errorDescription); 
	Message = t;

	SysFreeString(errorDescription);
	errorInfo->Release();

	switch(mode)
	{
		case BothFilePrint:

		case onlyFile:

		gLog.LogMessage(funcName, Message);

		if(mode == onlyFile)
			break;

	case onlyPrint:

		cout << funcName << ":" << Message << endl;

		break;

	}
}