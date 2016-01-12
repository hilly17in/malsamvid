#pragma once

class CmdLineProcessor
{
private:
	std::vector<std::string> vwArgs;
	int argc;
	
public:
	CmdLineProcessor(void);
	~CmdLineProcessor(void);
	void Process(int argc, char **argv);
	unsigned int CmdLineProcessor::HasSwitch(std::string fnArg, std::string &value);
};

