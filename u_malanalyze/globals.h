# pragma once

// this will hold the getlasterror() number
extern int last_errno ;

extern HANDLE ghMADevice;

extern PROCESS_INFORMATION gProcessInfo;

typedef struct _tag_feature_vector
{
	int value;
	int count;
} FV;

class data
{
private:
	wstring filepath;
		
public:

	data()
	{
	}

	data(wstring str) 
	{ 
		filepath = str;
	}

	void replace(wstring wstr)
	{
		filepath = wstr; 
	}

	friend wostream& operator <<(wostream& outs, data& d)
	{ 
		outs << d.filepath << endl; 
		return outs;
	}

	wstring getfilepath()
	{
		return filepath;
	}

	int operator==(data d2)
	{
		if(filepath == d2.filepath)
		{
			//cout<<"matching";		
			return 1;
		}	
		else{
			//cout<<"not matching";	
			return 0;
		}
	}
};


//extern vector<data> dataCreate;
extern vector<hashinfo> dataCreate;

extern vector<data> dataOpen ;

extern vector<data> dataRead;

extern vector<data> dataWrite;

extern vector<data> dataSetSec ;

extern vector<data> dataDel;

extern vector<data> dataIoctl;

extern vector<data> datatimestamp;

extern vector<data> dataCreateRegKey;

extern vector<data> dataSetValueKey;

extern vector<data> dataDeleteKey;

extern vector<data> dataOpenKey;

extern vector<data> dataEnumKey;

extern vector<data> dataQueryKey;

extern vector<data> dataEnumValueKey;

extern vector<data> dataQueryValueKey;

extern vector<data> dataProcess;

extern vector<data> dataTcpcon;

extern vector<wstring> notrepeat;

extern vector<wstring> gQuery;

extern vector<int> dataPid;

extern data dc ;
extern data dop;
extern data dr;
extern data dw;
extern data ds;
extern data dd;
extern data di;
extern data dt;
extern data dproc;
extern data tc;

extern data dcrk;
extern data dsek;
extern data delk;
extern data openk;
extern data enumk;
extern data enumv;
extern data queryk;
extern data queryv;

extern wfstream file;

extern int		gSusp;
extern FV		FeatureVector[];
extern wstring	prog;
extern char		filename_pktdump[] ;
extern wchar_t  malwarename[];
extern hashinfo h_prog;
extern WCHAR 	TempMD5[];