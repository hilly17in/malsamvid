#include "StdAfx.h"


CmdLineProcessor::CmdLineProcessor(void)
{
}


CmdLineProcessor::~CmdLineProcessor(void)
{
}


void CmdLineProcessor::Process(int argc, char **argv)
{
	int cnti = 0;

	if(argc > 1)
	{
		this->argc = argc;

		while(cnti < argc){
	
			this->vwArgs.push_back(argv[cnti]);

			++cnti;
		}
	}
	else
	{
		CmdException e;
		e.SetInformation("Not Enough Arguments to Proceed.");
		throw e;
	}

}

unsigned int CmdLineProcessor::HasSwitch(std::string option, std::string &value)
{
	int cnti = 0;

	while(cnti < argc)
	{
		if(option == vwArgs.at(cnti))
		{
			value = vwArgs.at(++cnti);
			return 1;
		}
			
		cnti++;
	}

	return 0;
}