# include "stdafx.h"



// Remove New files. this functions traverses the new creaded files' vector and 
// then copies them to analyst folder
//
//VOID
//RemoveNewFiles()
//{
//	vector<data>::iterator itr1 = dataCreate.begin() ;
//	wstring		temp;
//	WCHAR		Buffer[MAX_PATH];
//	static		int entered = 0;
//	
//
//	StringCbPrintf(Buffer, MAX_PATH * sizeof(WCHAR), L"%ws\\backup_files", dirpath);
//
//	if(!CreateDirectory(Buffer, NULL))
//	{
//		printf("%ws \nbackup direcory could not be created (%d)\n", Buffer, GetLastError());
//		if(GetLastError() != 183)
//			return ;
//	}
//
//	while(itr1 != dataCreate.end())
//	{
//		wchar_t newfilename[32];
//		wchar_t src[MAX_PATH];
//		wchar_t dst[MAX_PATH];
//		
//		temp = itr1->getfilepath();
//		extractfinalname(temp.c_str(), newfilename);	
//
//		StringCbPrintf(dst, MAX_PATH * sizeof(WCHAR), L"%ws\\%ws", Buffer, newfilename);
//
//		convertwithdrive(temp.c_str(), temp.length(), src);
//
//		if(!CopyFile(src, dst, TRUE))
//			printf("src = %ws \n dst = %ws \n error : %d \n", src, dst, GetLastError());
//		
//		++itr1;
//	}
//
//	return ;
//}
// 
// RemoveNewFiles

// Remove New files. this functions traverses the new creaded files' vector and 
// then copies them to analyst folder
//
VOID
RemoveNewFiles()
{
	vector<hashinfo>::iterator itr1 = dataCreate.begin() ;
	WCHAR		TempFilePath[MAX_IMAGE_PATH];	//wstring		temp;
	WCHAR		Buffer[MAX_PATH];
	static		int entered = 0;
	

	StringCbPrintf(Buffer, MAX_PATH * sizeof(WCHAR), L"%ws\\backup_files", dirpath);

	if(!CreateDirectory(Buffer, NULL))
	{
		if(GetLastError() != ERROR_ALREADY_EXISTS)
		{
			printf("%ws \nbackup direcory could not be created (%d)\n", Buffer, GetLastError());
			return ;
		}
	}

	while(itr1 != dataCreate.end())
	{
		wchar_t newfilename[32];
		wchar_t src[MAX_PATH];
		wchar_t dst[MAX_PATH];
		
		itr1->GetFilePath(TempFilePath);
		extractfinalname(TempFilePath, newfilename);	

		StringCbPrintf(dst, MAX_PATH * sizeof(WCHAR), L"%ws\\%ws", Buffer, newfilename);

		if(!CopyFile(TempFilePath, dst, TRUE))
			printf("src = %ws \n dst = %ws \n error : %d \n", src, dst, GetLastError());
		else
			itr1->CalcAllHashes();
		
		++itr1;
	}

	return ;
}
// 
// RemoveNewFiles

int findinnotrepeat(wstring str)
{
	unsigned int cnt = 0;
	int i = 0 ;

	//wcout<<L"bbaa"<<endl;
	while(cnt < notrepeat.size())
	{
		i = str.find(notrepeat[cnt]);
		//cout<<" i is "<< i << endl;
		if (i >= 0)
			return 1 ;

		cnt++;
	}
	//wcout<<L"aaddffg"<<endl;
	return 0;
}

VOID
DumpQuery(vector<wstring> *datalist, wstring wstr)
{
	vector<wstring>::iterator itr1 = datalist->begin() ;
	vector<wstring>::iterator itr2 = datalist->begin() ;

	while(itr1 != datalist->end())
	{
		itr2 = itr1 + 1;
		while(itr2 != datalist->end())
		{
			if(*itr1 == *itr2)
			{
				//cout<<"matching \n";
				itr2 = datalist->erase(itr2);
			}
			else
			{
				//cout<<" not matching \n";
				++itr2;
			}
		}

		++itr1;
	} // first while for create

	unsigned int	i = 0;

	file<<wstr<<endl<<L"---------------------------------------------------"<<endl;

	itr1 = datalist->begin() ;
	while(itr1 != datalist->end())
	{
		file<<*itr1<<endl;
		++itr1;
	}
	
	file<<L"/*========================================*/"<<endl<<endl<<endl<<endl;

}

//
// DumpFile
//
VOID
DumpFile(vector<data> *datalist, wstring wstr)
{
	vector<data>::iterator itr1 = datalist->begin() ;
	vector<data>::iterator itr2 = datalist->begin() ;

	while(itr1 != datalist->end())
	{

		itr2 = itr1 + 1;
		while(itr2 != datalist->end())
		{
			if(*itr1 == *itr2)
			{
				//cout<<"matching \n";
				itr2 = datalist->erase(itr2);
			}
			else
			{
				//cout<<" not matching \n";
				++itr2;
			}
		}

		++itr1;
	} // first while for create

	unsigned int	i = 0;

	file<<wstr<<endl<<L"---------------------------------------------------"<<endl;

	itr1 = datalist->begin() ;
	while(itr1 != datalist->end())
	{
		file<<itr1->getfilepath()<<endl;
		++itr1;
	}
	
	file<<L"/*========================================*/"<<endl<<endl<<endl<<endl;

}
//
// DumpFile
//


//
// DumpFile
//
VOID
DumpFile(vector<hashinfo> *datalist, wstring wstr)
{
	vector<hashinfo>::iterator itr1 = datalist->begin() ;
	vector<hashinfo>::iterator itr2 = datalist->begin() ;
	
	// skipping the duplicate check as creating same new files in same location 
	// carries very low probability

	unsigned int	i = 0;

	file<<wstr<<endl<<L"---------------------------------------------------"<<endl;

	itr1 = datalist->begin() ;
	while(itr1 != datalist->end())
	{
		WCHAR	TempFilePath[MAX_IMAGE_PATH];
		WCHAR	TempMD5[WMD5LEN]=L"";
		itr1->GetFilePath(TempFilePath);
		itr1->GetMD5(TempMD5);

		file<<TempFilePath<<"( "<<TempMD5<<" )"<<endl;
		++itr1;
	}
	
	file<<L"/*========================================*/"<<endl<<endl<<endl<<endl;
}
//
// DumpFile
//

VOID DumpAnalysis()
{
	unsigned int	cnt = 0;
	int				n = 0;
	int				i = 0;
	wstring			tempstr;
	int				len;
	WCHAR			*dst;
	
	// whether program deletes self
	for(cnt=0; cnt < dataDel.size();cnt++)
	{
		tempstr = dataDel[cnt].getfilepath();
		len = tempstr.length();
		dst = new WCHAR[len+1];
		convertwithdrive(tempstr.c_str(), len, dst);

		//i=tempstr.find(prog);
		//wcout<<L"tempstr is = "<<tempstr<<L"\n prog is = \n"<<prog<<endl;
		//if(i>=0)
		
		if(!wcscmp(dst, prog.c_str() ) )
		{
			file<<endl<<L"delets the self"<<endl;
			FeatureVector[4].value = 7; //gSusp = gSusp + 3 ;
			FeatureVector[4].count = 2;
			break;
		}

	} // for outer
				
	// if deletes many registry keys
	int size = dataDeleteKey.size();
	if(size>15)
	{
		file<<endl<<L"deletes too many registry keys"<<endl;
		FeatureVector[12].value=3;//++gSusp;
		FeatureVector[12].count = 1;
	}
							
	// if deletes many files
	size = dataDel.size();
	int deletesize = 0;
	if(size>0)
	{
		int x = 0;
		wstring delkey;

		while(x < size) {
			delkey = dataDel[x].getfilepath();

			if( (delkey.find(L"\\windows\\temp") == string::npos) && 
				(delkey.find(L"Microsoft\\Windows\\Temporary Internet Files") == string::npos) 
				)
				deletesize++;	

			x++;
		}
	}

	if(deletesize > 5){
		file<<endl<<L"deletes too many files"<<endl;
		FeatureVector[13].value = 3;//++gSusp;
		FeatureVector[13].count = 1;
	}	

	printf("check three done \n");
	// if changes sec perms of many files
	size = dataSetSec.size();
	if(size>10)
	{
		file<<endl<<L"changes the security permissions of too many files"<<endl;
		FeatureVector[14].value = 3;//++gSusp;
		FeatureVector[14].count = 1;
	}

	// if changes timestamps of many files
	size = datatimestamp.size();
	if(size>10)
	{
		file<<endl<<L"changes the time stamps of too many files"<<endl;
		FeatureVector[15].value = 3;//++gSusp;
		FeatureVector[15].count = 1;
	}

	size = dataTcpcon.size();
	if(size > 0)
	{
		file<<L"Program listens on a port!"<<endl;
		FeatureVector[6].value = 5; //gSusp = gSusp + 2 ;
		FeatureVector[6].count = 2;
	}

	printf("before mesg from det \n");
	MessageFromDetour();

} // DumpAnalysis

VOID DumpAnalysis_Newfiles()
{
	unsigned int cnt = 0;
	WCHAR temp[WMD5LEN];

	for(cnt=0; cnt < dataCreate.size();cnt++)
	{
		dataCreate[cnt].GetMD5(temp);
	
		if( !_wcsicmp(temp, TempMD5) )
		{
			file<<endl<<"Creates copy of itself"<<endl;
			FeatureVector[11].value = 5;//gSusp = gSusp + 5;
			FeatureVector[11].count = 2;
			break;
		}


	} // for outer

}



VOID DumpAnalysis_Readfile()
{
	wstring temp[] = { L"etc\\hosts",
					   L"etc\\services",
					   L"etc\\lmhosts.sam",
					   L"system32\\dssec.dat"
					   L"system32\\oembios.dat"
					   L"system32\\config\\sam"
	};

	wstring desc[] = { L"reads dns mappings",
					   L"reads services (port num) to TCPIP Services",
					   L"reads name resolution mappings",
					   L"read the Active Directory Settings - may get the user name, email ids",
					   L"reads the oem license information",
					   L"trying to read password files"
	};

	unsigned int cnt = 0;
	int n=0;
	int i = 0;
	wstring tempstr;

	for(cnt=0; cnt < dataRead.size();cnt++)
	{
		tempstr = dataRead[cnt].getfilepath();
		
		for(n=0; n < 6; n++)
		{
			i = tempstr.find(temp[n]);
			
			if(i>=0)
			{
				file<<endl<<endl<<desc[n]<<endl<<endl;
				FeatureVector[0].value = 2;  //++gSusp;
				FeatureVector[0].count++;
				break;
			}
		} // for inner
	} // for outer

} // DumpAnalysis_Readfile


VOID DumpAnalysis_Writefile()
{
	unsigned int cnt = 0;
	int n = 0;
	int i = 0;
	int k = 0;
	int found = -1;
	wstring extension;
	int writestoexes = 0;

	wstring tempstr;
	wstring extexe[]= { L".exe",
						L".dll",
						L".tmp",
						L".pl",
						L".py",
						L".sys",
						L".cpl",
						L".ocx",
						L".msi"
	};
	
	wstring temp[] = { L"etc\\hosts",
					   L"etc\\services",
					   L"etc\\lmhosts.sam",
					   L"system32\\dssec.dat",
					   L"Windows\\System32\\winevt\\",
					   L"system32\\config\\sam",
					   L"\\Windows\\"
	};

	wstring desc[] = { L"adds dns mappings",
					   L"adds services (port num) to TCPIP Services",
					   L"adds name resolution mappings",
					   L"may change the Active Directory Settings",
					   L"writing to event logs",
					   L"trying to write to password files",
					   L"Writing to windows directory"
	};

	for(cnt=0; cnt < dataWrite.size();cnt++)
	{
		tempstr = dataWrite[cnt].getfilepath();
		
		for(n=0; n < 7; n++)
		{
			i = tempstr.find(temp[n]);
			
			if(i>=0)
			{
				file<<endl<<endl<<desc[n]<<endl<<endl;
				FeatureVector[2].value = 7;  //++gSusp;
				FeatureVector[2].count++;
				break;
			}
		} // for inner

		i = -1;

		for(k=0; k < 9; k++)
		{
			found = tempstr.find_last_of(L".");
			extension = tempstr.substr(found+1);
			i = extension.compare(extexe[k]);

			if(i == 0)
			{	
				++writestoexes;
				FeatureVector[0].value = 2;  //++gSusp;
				FeatureVector[0].count++;
				break;	// can be removed
			}
		}
	} // for outer

	if(writestoexes)
		file<<"writes to "<<writestoexes<<" executables"<<endl;

}

VOID DumpAnalysis_SetValue()
{	
	wstring temp[] = { L"Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced Hidden", /*1*/
					   L"Microsoft\\Windows\\CurrentVersion\\Policies\\system\\DisableRegistryTools",/*2*/
					   L"Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile\\AuthorizedApplications\\List",/*3*/
					   L"Microsoft\\Windows\\CurrentVersion\\policies\\system\\EnableLUA",/*4*/
					   L"Microsoft\\Windows\\CurrentVersion\\Run",/*5*/
					   L"Microsoft\\Windows\\CurrentVersion\\explorer\\Browser Helper Objects",/*6*/
					   L"Microsoft\\Internet Explorer\\URLSearchHooks",/*7*/
					   L"Microsoft\\Internet Explorer\\Toolbar",/*8*/
					   L"Microsoft\\Internet Explorer\\Extensions",/*9*/
					   L"Services\\WinSock2\\Parameters\\Protocol_Catalog",/*10*/
					   L"Microsoft\\Windows NT\\CurrentVersion\\Winlogon userinit",/*11*/
					   L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",/*12*/
					   L"Control\\Safeboot",/*13*/
					   L"Microsoft\\Windows NT\\CurrentVersion\\SvcHost",/*14*/
					   L"Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\ShowSuperHidden",/*15*/
					   L"Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\SuperHidden",/*16*/
					   L"Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWALL\\CheckedValue",/*17*/
					   L"Microsoft\\Windows NT\\CurrentVersion\\Winlogon shell",/*18*/
					   L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32",/*19*/
					   L"SOFTWARE\\Microsoft\\Security Center AntiVirusDisableNotify", /*21*/
					   L"SOFTWARE\\Microsoft\\Security Center AntiVirusOverride", /*22*/
					   L"SOFTWARE\\Microsoft\\Security Center FirewallDisableNotify", /*23*/
					   L"SOFTWARE\\Microsoft\\Security Center FirewallOverride",/*24*/
					   L"SOFTWARE\\Microsoft\\Security Center UpdatesDisableNotify", /*25*/
					   L"SOFTWARE\\Policies\\Microsoft\\Windows Defender DisableAntiSpyware", /*26*/
					   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\DisableTaskMgr", /*27*/
					   L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MountPoints2", /*20*/
	};

	wstring desc[] = { L"Toggles hidden files so that user cant see hidden files",
					   L"Disables registry editing tool",
					   L"Adding rules so that it can bypass windows firewall",
					   L"An possible attempt to change UAC settings",
					   L"Adds a program which will run automatically on next login",
					   L"Adding BHOs in IE",
					   L"Adding URL Search hooks in IE",
					   L"Installing tool bar in IE",
					   L"Installing extensions in IE",
					   L"Installing winsock provider and changing the chain",
					   L"Changes winlogon parameters",
					   L"Changes the internet settings",
					   L"Adds a program to safeboot",
					   L"Install hidden services in svchost.exe",
					   L"Sets not to show the operating system files",
					   L"Sets not to show the super hidden files",
					   L"Sets Folder Options to not show hidden files",
					   L"Adding a program which will be also running with explorer.exe",
					   L"Registering new codec DLL.",
					   L"Antivirus notification settings are changed.",
					   L"Setting are changed so that windows doesnt monitor the antivirus program.",
					   L"Firewall notification settings are changed.",
					   L"Setting are changed so that windows doesnt monitor the firewall program.",
					   L"Windows update notification settings are changed.",
					   L"Disables Windows Defender AntiSpyware",
					   L"Disables Task Manager",
					   L"Adding new mount points.",
	} ;

	wstring strservice1 = L"SYSTEM\\ControlSet" ;
	wstring strservice2 = L"Services" ;
	int a, b;
	int bservice=0;

	int n = 0;
	int i = 0;
	int j = 0;
	unsigned int cnt = 0;

	wstring tempstr;
	
	for(cnt=0; cnt < dataSetValueKey.size();cnt++)
	{
		tempstr = dataSetValueKey[cnt].getfilepath();

		for(n=0; n < MAX_SETVAL_BEHAVIOR; n++)
		{
			i = tempstr.find(temp[n]);
			if(i>=0)
			{
				if(!findinnotrepeat(desc[n]))
				{
					file<<endl<<desc[n];
					notrepeat.push_back(desc[n]);
				}

				wcout<<endl<<tempstr<<endl;
				FeatureVector[3].value = 7;  //++gSusp;
				FeatureVector[3].count++;

				break;
			}
		} // for inner
		
	if(bservice == 0)
	{
		a = tempstr.find(strservice1);
		b = tempstr.find(strservice2);
		if( (a>=0) && (b>=0) )
		{
			file<<endl<<L"install services or drivers";
			FeatureVector[1].value = 2;  //++gSusp;
			FeatureVector[1].count++;
			bservice=1;
		}
	}

	} // for outer
}


DWORD DisplayReport()
{
	// TODO
	// 1. take another memory snapshot and the compare.
	// 2. add hash of given file and hashes of all newly created files by the malware

	RemoveNewFiles();
	
	// Calcualte hash of program under analysis
	
	// formatting if any
	wchar_t	name[MAX_PATH];

	StringCbPrintf(name, MAX_PATH-1, L"%ws\\report_%s.txt", dirpath, malwarename);
	file.open(name, ios::out);

	file<<"****************************************"<<endl<<endl;
	file<<"		analysis for "<<prog<<endl<<endl;
	file<<"		( "<<TempMD5<<" )"<<endl;
	file<<"****************************************"<<endl<<endl;

	/*==========datcreate===============*/
	DumpFile(&dataCreate, L"new files created  ");
	cout<<"Please wait..."<<"5%";

	/*==========dataopen===============*/
	DumpFile(&dataOpen, L"opened files ");
	cout<<'\r'<<"Please wait..."<<"10%";

	/*==============dataread=================*/
	DumpFile(&dataRead, L"these files are read ");
	cout<<'\r'<<"Please wait..."<<"15%";

	/*==============datawrite===================*/
	DumpFile(&dataWrite, L"these files are written into  ");
	cout<<'\r'<<"Please wait..."<<"20%";

	/*=============datasetsec================*/
	DumpFile(&dataSetSec, L"these files' sec perms changed ");
	cout<<'\r'<<"Please wait..."<<"25%";

	/*===========datadel===============*/
	DumpFile(&dataDel, L"these files are deleted ");
	cout<<'\r'<<"Please wait..."<<"30%";

	/*===========dataioctl===============*/
	DumpFile(&dataIoctl, L"these files are sent ioctl ");
	cout<<'\r'<<"Please wait..."<<"35%";

	/*===========datatimestamp===============*/
	DumpFile(&datatimestamp, L"these files' timestamps changed ");
	cout<<'\r'<<"Please wait..."<<"40%";
	
	/*===========datacreateregkey===============*/
	DumpFile(&dataCreateRegKey, L"these registry keys are created ");
	cout<<'\r'<<"Please wait..."<<"45%";

	/*===========datasetvaluekey===============*/
	DumpFile(&dataSetValueKey, L"these registry keys values set ");
	cout<<'\r'<<"Please wait..."<<"50%";

	/*===========datadeletekey===============*/
	DumpFile(&dataDeleteKey, L"these registry keys are deleted ");
	cout<<'\r'<<"Please wait..."<<"55%";

	/*==========Opens Registry Keys ==============*/
	DumpFile(&dataOpenKey, L"these registry keys are opened ");
	cout<<'\r'<<"Please wait..."<<"56%";

	/*==========Enumerates Registry Keys ==============*/
	DumpFile(&dataEnumKey, L"these registry keys are enumerated ");
	cout<<'\r'<<"Please wait..."<<"57%";

	/*==========Enumerates Registry Values ==============*/
	DumpFile(&dataEnumValueKey, L"these registry Values are enumerated ");
	cout<<'\r'<<"Please wait..."<<"57%";

	
	/*==========Query Registry Keys ==============*/
	DumpFile(&dataQueryKey, L"these registry keys are queried ");
	cout<<'\r'<<"Please wait..."<<"58%";

	/*==========Query Registry Values ==============*/
	DumpFile(&dataQueryValueKey, L"these registry Values are queried ");
	cout<<'\r'<<"Please wait..."<<"58%";

	/*=============data Process =============*/	
	DumpFile(&dataProcess, L"these processes are created ");
	cout<<'\r'<<"Please wait..."<<"60%";

	/* == tcp connections ==*/
	DumpFile(&dataTcpcon, L"These are the connections made by the process");
	cout<<'\r'<<"Please wait..."<<"65%";

	// analyze pcap file for dns queries
	PCAP_Analysis();
	DumpQuery(&gQuery, L"Name Queries");
	cout<<'\r'<<"Please wait..."<<"70%";

	// now analyze the report...
	file<<endl<<endl<<L"some automated analysis"<<endl<<L"************************"<<endl;

	DumpAnalysis_SetValue();
	cout<<'\r'<<"Please wait..."<<"75%";

	DumpAnalysis_Newfiles();

	DumpAnalysis_Writefile();
	cout<<'\r'<<"Please wait..."<<"80%";

	DumpAnalysis_Readfile();
	cout<<'\r'<<"Please wait..."<<"85%";

	DumpAnalysis();
	cout<<'\r'<<"Please wait..."<<"90%";


	// decide whether program is suspicious
	double score = 0.0;
	unsigned int arrcount = 0;
	double et = 0.0;

	ofstream global_op;
	global_op.open("db_weka.csv", ios_base::app);

	while(arrcount < 17) {
		global_op<<FeatureVector[arrcount].value<<",";
		if(FeatureVector[arrcount].value == 0)
			et = 0.0 ;
		else
			et = pow((double)FeatureVector[arrcount].value, FeatureVector[arrcount].count);
		
		score = score + et;
		
		//cout<<FeatureVector[arrcount].value<<":"<<FeatureVector[arrcount].count<<":"<<et<<":"<<score<<endl;
		
		arrcount++;
	}

	double final = 0.0;
	final = sqrt (score);

	file<<endl<<"the final score: "<<final<<endl;
	global_op<<final<<endl;
	
	//if(gSusp <= 0)
	//	file<<endl<<endl<<"seems to be a genuine program"<<endl;
	//else if( (gSusp > 0) && (gSusp < 5))
	//	file<<endl<<L"*********"<<endl<<L"a little suspicious program"<<endl<<L"****************";
	//else if(gSusp < 10)
	//	file<<endl<<L"*********"<<endl<<L"a suspicious program"<<endl<<L"****************";
	//else
	//	file<<endl<<L"*********"<<endl<<L"a very very suspicious program"<<endl<<L"****************";


	file.close();
	cout<<'\r'<<"Malware analysis is completed !"<<endl;
	/*--------------------- finished -----------------*/


	//---------------------
	// createprocess notepad.exe with report name
	DWORD	status = SUCCESS;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	wchar_t Fullpath[MAX_PATH] ;
	wchar_t currDir[MAX_PATH];
	
	memset(Fullpath, 0, MAX_PATH * sizeof(WCHAR));
	memset(currDir, 0, MAX_PATH * sizeof(WCHAR));
	
	GetCurrentDirectory(MAX_PATH, currDir);
	StringCbPrintf(Fullpath, MAX_PATH * sizeof(WCHAR), L"notepad.exe %ws", name);

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	ZeroMemory(&gProcessInfo, sizeof(PROCESS_INFORMATION));
		
    // Start the child process. 
    if( !CreateProcess( 
		NULL,				// No module name (use command line)
        Fullpath,			// Command line
        NULL,				// Process handle not inheritable
        NULL,				// Thread handle not inheritable
        FALSE,				// Set handle inheritance to FALSE
		0,					// No creation flags
        NULL,				// Use parent's environment block
        NULL,				// Use parent's starting directory 
        &si,				// Pointer to STARTUPINFO structure
        &pi )				// Pointer to PROCESS_INFORMATION structure
    ) 
    {
		printf( "CreateProcess with notepad to display report has failed (error: %d)\n", GetLastError() );
        return ERROR_PROCCREATE_FAIL;
    }

	return 0;
} // DisplayReport