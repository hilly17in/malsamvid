// MalSamvid.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, char* argv[])
{
	cout << "Welcome to MalSamvid" << std::endl;
	
	CmdLineProcessor	clpObj;
	std::string			wsConfigFile;

	// parse command line option
	// we are looking for only configuration file path
	// but its clean code !
	//
	try {
		clpObj.Process(argc, argv);
		if(!clpObj.HasSwitch("-c", wsConfigFile)) {
			std::cout << "No supported options provided to proceed" << std::endl ; 
			std::cout << "usage: malsamvid.exe -c <config-file>" << endl;
			return 0;
		}

	}
	catch(CmdException &e)
	{
		cout << e.GetInformation() << std::endl;
	}

#ifdef _VERBOSE
	std::cout<<wsConfigFile<<endl;
#endif

	// Now parse the configuration file
	// vm related setting and common setting i.e sample related paths.
	// Read the file. If there is an error, report it and exit.
	//
	unsigned int retVal = MVID_OK;
	retVal = ReadSettings(wsConfigFile);
	if(retVal != MVID_OK) {
		LogError(BothFilePrint, "ReadSettings", retVal);
		return 0;
	}

	// Write header in log.
	
	// create a thread for each analyzer
	unsigned int iThrCount = 0;
	std::vector<VMAnalyzer>::iterator itr;
	sVMAnalyzer vmDetail;
	CListDir sample_files;

	// enumerate directory to get all files.
	if(csettings.bDir)
	{
		sample_files.SetDirectoryPath(csettings.m_sampleDir);
		sample_files.listDir("");
	}
	else
	{
		sample_files.filenames.push_back(csettings.m_samplePath);
	}


	// Create Two mutex objects for synchronization
	if (!CreateSyncObjects())
	{
		return 0;
	}

	// return values of calls when we wait for these objects
	DWORD dwWaitResult = 0;
	DWORD dwWaitResult2 = 0;

	// now start feeding file one by one
	// the killer loop
	//
	unsigned int iFileCount  = 0;
	while(iFileCount<sample_files.filenames.size())
	{

		// two details
		// file details, and vm details
	
		// go through iterator.
		for (itr = vmAnalyzers.begin() ; itr != vmAnalyzers.end(); ++itr) {
	

			memset(&vmDetail, '\0', sizeof(vmDetail));
			itr->GetAnalyzerDetails(vmDetail);
			vmDetail.m_FilePath = sample_files.filenames[iFileCount];
			unsigned int x = vmDetail.MatchEmuSW();
			switch(x)
			{
			case EMU_VMWARE:
				// for vmware
				dwWaitResult = WaitForSingleObject(ghMutexVMWare, INFINITE);
				if(dwWaitResult == WAIT_OBJECT_0)
					VMWareAnalysis(vmDetail);
						
				break;

			case EMU_VIRTUALBOX:
				// for vbox
				dwWaitResult2= WaitForSingleObject(ghMutexVbox, INFINITE);
				if(dwWaitResult2 == WAIT_OBJECT_0)
					VBoxAnalysis(vmDetail);

				break;
			}
			
		} // for



		iFileCount++;
	}


	CloseHandle(ghMutexVMWare);
	CloseHandle(ghMutexVbox);
	
	if(hThreadArray[0] != NULL)
		WaitForSingleObject(hThreadArray[0], INFINITE);

	if(hThreadArray[1] != NULL)
		WaitForSingleObject(hThreadArray[1], INFINITE);

	// go through sample info
	// distribute to each of worker thread

	return 0;
}

