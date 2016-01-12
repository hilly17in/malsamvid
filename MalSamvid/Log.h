#pragma once

typedef enum
{
	onlyPrint,
	onlyFile,
	BothFilePrint
} eLogMode;

class CLog
{
	ofstream logfile ;
public:
	CLog(void);
	~CLog(void);
	void LogMessage(string funcName, const char *mesg);
};

void LogError(eLogMode mode, string funcName, unsigned int iError);
void LogErrorVbox(eLogMode mode, string funcName, HRESULT rc);

extern CLog gLog;