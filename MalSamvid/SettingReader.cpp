#include "stdafx.h"

std::vector<VMAnalyzer> vmAnalyzers;
CommonSettings	csettings;
std::vector<DWORD> vThreadId;
std::string emu_vmware = "VMWare";
std::string emu_vbox =   "VirtualBox";

VMAnalyzer::VMAnalyzer(void)
{
}

VMAnalyzer::~VMAnalyzer(void)
{
}

void VMAnalyzer::SetAnalyzerDetails(std::string argName,
					   std::string argEmuSw,
					   std::string argVMPath,
					   std::string argUname,
					   std::string argPasswd,
					   std::string &argWinDir,
					   std::string &argSnapshot)
{
	m_name = argName;
	m_emulatorSW = argEmuSw;
	m_VMPath = argVMPath;
	m_uname = argUname;
	m_password = argPasswd;
	m_Snapshot = argSnapshot;
	m_WinDir = argWinDir;
}

//void VMAnalyzer::GetAnalyzerDetails(std::string &argName,
//					   std::string &argEmuSw,
//					   std::string &argVMPath,
//					   std::string &argUname,
//					   std::string &argPasswd,
//					   std::string &argWinDir,
//					   std::string &argSnapshot)
//{
//	argName = m_name;
//	argEmuSw = m_emulatorSW;
//	argVMPath = m_VMPath;
//	argUname = m_uname;
//	argPasswd = m_password;
//	argSnapshot = m_Snapshot;
//	argWinDir = m_WinDir;
//}

void VMAnalyzer::GetAnalyzerDetails(sVMAnalyzer &argVMDetail)
{
	argVMDetail.m_name = this->m_name;
	argVMDetail.m_emulatorSW = this->m_emulatorSW;
	argVMDetail.m_VMPath = this->m_VMPath;
	argVMDetail.m_uname = this->m_uname;
	argVMDetail.m_password = this->m_password;
	argVMDetail.m_Snapshot = this->m_Snapshot;
	argVMDetail.m_WinDir = this->m_WinDir;
}
//****************************************************************************************

sVMAnalyzer::sVMAnalyzer()
{
	m_name.clear();
	m_emulatorSW.clear();
	m_VMPath.clear();
	m_uname.clear();
	m_password.clear();
	m_Snapshot.clear();
	m_WinDir.clear();
	m_FilePath.clear();
}


sVMAnalyzer::~sVMAnalyzer()
{

}


unsigned int sVMAnalyzer::MatchEmuSW()
{
	if(this->m_emulatorSW == emu_vmware)
	{
		
		return EMU_VMWARE;
	}
	if(this->m_emulatorSW == emu_vbox)
		return EMU_VIRTUALBOX;

	return 0;
}

//****************************************************************************************

CommonSettings::CommonSettings(void)
{
	bDir = FALSE;
	bPath = TRUE;
}

CommonSettings::~CommonSettings(void)
{
}

void CommonSettings::SetParams(std::string argSampleDir,
			  std::string argSamplePath,
			  std::string argOutputDir,
			  std::string argDataset)
{
	m_samplePath = argSamplePath;
	m_sampleDir = argSampleDir;
	m_outputDir = argOutputDir;
	m_analysisDataset = argDataset;

}

void CommonSettings::GetParams(std::string &argSampleDir,
			  std::string &argSamplePath,
			  std::string &argOutputDir,
			  std::string &argDataset)
{
	argSampleDir = m_sampleDir;
	argSamplePath = m_samplePath;
	argOutputDir = m_outputDir;
	argDataset = m_analysisDataset;

}

unsigned int
	ReadSettings(std::string configFile)
{
		
	Config			cfg;
	
	try
	{
		cfg.readFile(configFile.c_str());
	}
	catch(const FileIOException &fioex)
	{
		std::cerr << "I/O error while reading file." << std::endl;
		return CONFIG_FILE_READ_FAILED;
	}
	catch(const ParseException &pex)
	{
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
		return(CONFIG_FILE_READ_FAILED);
	}


 	// Get the store name.
  	try
	{
		std::string name = cfg.lookup("name");
		//cout << "Store name: " << name << endl << endl;
	}
	catch(const SettingNotFoundException &nfex)
	{
		LogError(onlyFile, "ReadSettings: Store Name Not found", 0);
		return CONFIG_NONAME_SETTING;
	}

	const Setting& root = cfg.getRoot();

	
	// Output a list of all analyzers in congiguration.
	try
	{
		const Setting &analyzer = root["analyzers"]["analyzer_configurations"];
		int count = analyzer.getLength();
		VMAnalyzer vmTemp;

		for(int i = 0; i < count; ++i)
		{
		  const Setting &config = analyzer[i];

		  // Only output the record if all of the expected fields are present.
		  string name, emu_sw, vmpath, uname, passwd;
		  string windir, revertsnap;

		  if(!(config.lookupValue("name", name)
			   && config.lookupValue("emulator_sw", emu_sw)
			   && config.lookupValue("vm_path", vmpath)
			   && config.lookupValue("username", uname)
			   && config.lookupValue("password", passwd)
			   && config.lookupValue("windows_dir", windir)
			   && config.lookupValue("revert_snapshot", revertsnap)))
			continue;

#ifdef _DEBUG
		  cout << "name \t: " << name << endl;
		  cout << "emulator_sw \t: " << emu_sw << endl;
		  cout << "vm_path \t: " << vmpath << endl;
		  cout << "username \t: " << uname << endl;
		  cout << "password \t: " << passwd << endl;
		  cout << "windows_dir\t: " << windir << endl;
		  cout << "revert_snapshot \t: " << revertsnap << endl;
#endif
		  vmTemp.SetAnalyzerDetails(name, emu_sw, vmpath, uname, passwd, windir, revertsnap);
		  vmAnalyzers.push_back(vmTemp);

		}
		cout << endl;
	  }
	  catch(const SettingNotFoundException &nfex)
	  {
		return CONFIG_NO_SETTINGS;
	  }

 
	 try
     {
		const Setting &common = root["analyzers"]["common_settings"];
		int count = common.getLength();

		const Setting &config = common[0];
			 
		// Only output the record if all of the expected fields are present.
		string samplePath, sampleDir, outputDir, analysisDataset;
			
		// check for sample path or sample dir
		if( !config.lookupValue("sample_path", samplePath) )
		{
			LogError(onlyFile, "Lookup Value- Sample Path Not found", 0);
			csettings.bPath = FALSE;

			if(!config.lookupValue("sample_dir", sampleDir)) {
				LogError(onlyFile, "Lookup Value- Sample Dir Not found", 0);
				return CONFIG_ERROR_NO_SAMPLE;
			}
			else
				csettings.bDir = TRUE;
		}

		
		// check for output dir and analysis dataset
		if(	(config.lookupValue("output_dir", outputDir))
			 && (config.lookupValue("analysis_dataset", analysisDataset)))
		{
			if(csettings.bDir)
				csettings.SetParams(sampleDir, "", outputDir, analysisDataset) ;
			else if(csettings.bPath)
				csettings.SetParams("", samplePath, outputDir, analysisDataset) ;
		}
		else
		{
			LogError(onlyFile, "Lookup Value- output Path Not found", 0);
			return CONFIG_ERROR_NO_OUTPUTDIR;
		}

		
		
	 }
	 catch(const SettingNotFoundException &nfex)
	 {
		return CONFIG_NO_COMMON_SETTINGS;
	 }

	 return MVID_OK;
}