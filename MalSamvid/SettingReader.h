#pragma once

// options for emulator software name
#define EMU_VMWARE		1
#define EMU_VIRTUALBOX	2
//extern std::string emu_vmware = "VMWare";
//extern std::string emu_vbox =   "VirtualBox";

// basic purpose of this is so that we can pass a structure argument to thread
//
class sVMAnalyzer
{
public:
	
	// virtual machine related details
	std::string m_name ;
	std::string m_emulatorSW;
	std::string m_VMPath;
	std::string m_uname;
	std::string m_password;
	std::string m_WinDir;			
	std::string m_Snapshot;		// Snapshot to which to revert after execution of malware

	// file related
	std::string	m_FilePath;

	unsigned int MatchEmuSW();
	sVMAnalyzer();
	~sVMAnalyzer();
} ;


class VMAnalyzer
{
private:
	std::string m_name ;		// analyzer name
	std::string m_emulatorSW;	// name of emulation software
	std::string m_VMPath;		// path of guest virtual machine
	std::string m_uname;		// user name of guest os
	std::string m_password;		// password of guest os
	std::string m_WinDir;		// path of windows directory in guest os	
	std::string m_Snapshot;		// Snapshot to which to revert after execution of malware

public:

	void SetAnalyzerDetails(
					   std::string argName,
					   std::string argEmuSw,
					   std::string argVMPath,
					   std::string argUname,
					   std::string argPasswd,
  					   std::string &argWinDir,
					   std::string &argSnapshot);

	//void GetAnalyzerDetails(std::string &argName,
	//				   std::string &argEmuSw,
	//				   std::string &argVMPath,
	//				   std::string &argUname,
	//				   std::string &argPasswd,
	//				   std::string &argWinDir,
	//				   std::string &argSnapshot);

	void GetAnalyzerDetails(sVMAnalyzer &argVMDetail);

	VMAnalyzer(void);
	~VMAnalyzer(void);
};


class CommonSettings
{
public:
	std::string m_samplePath;
	std::string m_sampleDir;
	std::string m_outputDir;
	std::string m_analysisDataset;

	bool bDir;
	bool bPath;

	void SetParams(std::string argSampleDir,
			  std::string argSamplePath,
			  std::string argOutputDir,
			  std::string argDataset);

	void GetParams(std::string &argSampleDir,
			  std::string &argSamplePath,
			  std::string &argOutputDir,
			  std::string &argDataset);

	CommonSettings(void);
	~CommonSettings(void);
};


extern std::vector<VMAnalyzer>	vmAnalyzers;
extern CommonSettings			csettings;
extern std::vector<DWORD>		vThreadId;

unsigned int
	ReadSettings(std::string configFile);
