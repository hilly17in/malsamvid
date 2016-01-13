# include "stdafx.h"

DWORD
MessageFromDetour()
{
	DWORD dwRetVal = SUCCESS;
	wifstream infile;
	wstring line;
	unsigned int reading = 0;
	
	infile.open(L"ignore.txt", ifstream::in);
	if(infile.is_open())
	{
		while(getline(infile, line)) {
			
			if(line.find(L"Enumerates-Processes") != string::npos){
				file<<endl<<line<<endl;
				FeatureVector[7].value = 3; //gSusp = gSusp + 3;
				FeatureVector[7].count = 1;
			}
			else if (line.find (L"read-process-memory") != string::npos){
				file<<endl<<line<<endl;
				FeatureVector[8].value = 3;//gSusp = gSusp + 3;
				FeatureVector[8].count = 1;
			}
			else if(line.find (L"write-process-memory") != string::npos) {
				file<<endl<<line<<endl;
				FeatureVector[9].value = 5; //gSusp = gSusp + 5;
				FeatureVector[9].count = 1;
			}
			else if(line.find(L"read-own-process-memory") != string::npos) {
					  reading = 1;
			}
			else if (line.find(L"write-own-process-memory") != string::npos)	{
				if(reading == 1) {
					FeatureVector[10].value = 5;
					FeatureVector[10].count = 1;
				}
			}

		} // while
	}
	else
	{
		printf("ignore file could not be opened \n");
	}

	return dwRetVal;
}