#pragma once

#define ADSREQUIRED	1

typedef
unsigned char 
(*PInspectFile)(
string wFilePath
 );

class CListDir
{
private:
	string			mDirPath;
	string			mExt;
	unsigned int	mExtLen;
	unsigned int	mNoExt;
	PInspectFile	mFunction;

	int enumfiles(string argDirPath);
	//int enumfiles_fn(string argDirPath);

public:
	vector<string> filenames;
	CListDir();	
	~CListDir();
	void SetDirectoryPath(string tmpPath);
	int listDir(string csvExt);
	//int listDir(PInspectFile InspectFunction);

};
