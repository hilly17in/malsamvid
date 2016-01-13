# include "stdafx.h"

int last_errno ;

int gSusp = 0;

FV FeatureVector[16];

HANDLE ghMADevice;

PROCESS_INFORMATION gProcessInfo;

//vector<data> dataCreate;

vector<hashinfo> dataCreate;

vector<data> dataOpen ;

vector<data> dataRead;

vector<data> dataWrite;

vector<data> dataSetSec ;

vector<data> dataDel;

vector<data> dataIoctl;

vector<data> datatimestamp;

vector<data> dataCreateRegKey;

vector<data> dataSetValueKey;

vector<data> dataDeleteKey;

vector<data> dataOpenKey;

vector<data> dataEnumKey;

vector<data> dataQueryKey;

vector<data> dataEnumValueKey;

vector<data> dataQueryValueKey;

vector<data> dataProcess;

vector<data> dataTcpcon;

vector<wstring> notrepeat;

vector<int> dataPid;

vector<wstring> gQuery;

data dc ;
data dop;
data dr;
data dw;
data ds;
data dd;
data di;
data dt;

data dcrk;
data dsek;
data delk;
data openk;
data enumk;
data enumv;
data queryk;
data queryv;

data dproc;
data tc;

wfstream	file ;
wstring		prog;
char		filename_pktdump[MAX_PATH] ;
wchar_t		malwarename[32];
hashinfo	h_prog;
WCHAR		TempMD5[WMD5LEN]=L"";