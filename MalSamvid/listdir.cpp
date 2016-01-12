
#include "stdafx.h"

CListDir::CListDir()
{
	//cout<<"this is constructor"<<endl
	mExtLen = 0;
	mNoExt = 0;
}
	
CListDir::~CListDir()
{
	//cout<<"this is destructor"<<endl
}

void CListDir::SetDirectoryPath(string tmpPath)
{
	mDirPath=tmpPath;
}

int CListDir::enumfiles(string argDirPath)
{
	WIN32_FIND_DATAA	FindFileData;
	HANDLE				hFind;
	string				dirpath;
	unsigned int		tmpLen;

	tmpLen = argDirPath.length();
	
	if(tmpLen > (MAX_PATH - 2) || !tmpLen)
		return -1;

	dirpath = argDirPath;
	dirpath.append("\\*");
	hFind = FindFirstFileA(dirpath.c_str(), &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE){
		DWORD error;
		error = GetLastError();
		fprintf(stdout, "FindFirstFile has failed.(%s)%d \n", argDirPath.c_str(), error);	
		if(error == 5)
			return 0;
		else
			return -1 ;
	}

	do {
	
		string tmpFilename(FindFileData.cFileName);

		// skip if these default directories
		if(tmpFilename == "." || tmpFilename == "..")
			continue;

		string recur;
		recur = argDirPath;
		recur.append("\\");
		recur.append(tmpFilename);

		//// if its a directory then call recursively
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY & ~FILE_ATTRIBUTE_HIDDEN) {
			if(enumfiles(recur) == -1)
				break;
		}
		else {					// if its a file then go ahead
			if(!mNoExt) {		
				int x=tmpFilename.rfind(mExt);
				int tmpLength = tmpFilename.length();			 // compare with requested extension
				if(x != tmpLength - mExtLen)
					continue;
			}
			filenames.push_back(recur);					// add to vector
		}

	}while(FindNextFileA(hFind, &FindFileData) != 0);
	

	return 0;

}

int CListDir::listDir(string csvExt)
{
	string			dirpath;
	unsigned int	dirpath_len;

	dirpath = mDirPath;
	dirpath_len = dirpath.length();
	if( dirpath_len > MAX_PATH-2)
		return -1;

	mExtLen = 0;
	mNoExt = 0;

	if(csvExt.size()) {
		mExt = csvExt;
		mExtLen = mExt.length();
	}
	else {
		mNoExt = 1;
	}

	return enumfiles(dirpath);
}

/*
int CListDir::enumfiles_fn(string argDirPath)
{
	WIN32_FIND_DATAA	FindFileData;
	HANDLE				hFind;
	string				dirpath;
	unsigned int		tmpLen;

	tmpLen = argDirPath.length();
	
	if(tmpLen > (MAX_PATH - 2) || !tmpLen)
		return -1;

	dirpath = argDirPath;
	dirpath.append("\\*");
	hFind = FindFirstFileA(dirpath.c_str(), &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE){

		DWORD error;
		error = GetLastError();
		fprintf(stdout, "FindFirstFile has failed.(%s)%d \n", argDirPath.c_str(), error);	
		if(error == 5)
			return 0;
		else
			return -1 ;
	}


	do {
		string tmpFilename(FindFileData.cFileName);

		// skip if these default directories
		if(tmpFilename == "." || tmpFilename == "..")
			continue;
		
		string recur;
		recur = argDirPath;

		//TODO
		// check for last char if \\

		recur.append("\\");
		recur.append(tmpFilename);
		

		// if its a directory then call recursively
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY & ~FILE_ATTRIBUTE_HIDDEN) {
			//cout<<recur<<endl;
			if(enumfiles_fn(recur) == -1)
				return -1;
		}
		else {			
			// if its a file then go ahead
			printf("file: %s \n", recur.c_str());
			if(mFunction(recur))
			{
				IsFilePacked(recur.c_str());
				//filenames.push_back(recur);
			}
		}

		printf("\r");

	}while(FindNextFileA(hFind, &FindFileData) != 0);
	

	return 0;

}
*/


/*
int CListDir::listDir(PInspectFile InspectFunction)
{
	string			dirpath;
	unsigned int	dirpath_len;

	dirpath = mDirPath;
	dirpath_len = dirpath.length();
	if( (dirpath_len > MAX_PATH-2) || (dirpath_len==0))
		return -1;

	if(InspectFunction == NULL)
		return -1;

	mFunction = InspectFunction;
	return enumfiles_fn(dirpath);

	return 0;
}
*/
	