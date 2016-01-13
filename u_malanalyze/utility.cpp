#include "stdafx.h"

wchar_t dirpath[MAX_PATH];

void extractthis(const wchar_t *test, wchar_t *output, int *pos)
{
	int index=0;
	int cnt = 0;
	int len = wcslen(test);
	while(index < len)
	{
		if(test[index] == '\\')
			if(++cnt == 3)
				break;

		output[index] = test[index];	
		index++;
	}

	*pos = index;

	printf("\n");
}

wchar_t GetDriveLetter(wchar_t *lpDevicePath)
{
	wchar_t d = 'A';
	
	while(d <= 'Z')
	{
		wchar_t szDeviceName[3] = {d,':','\0'};
		wchar_t szTarget[512] = {0};
	
		
		if(QueryDosDevice(szDeviceName, szTarget, 511) != 0)
			if(wcscmp(lpDevicePath, szTarget) == 0) 
				return d;
			
		d++;
	}
	return NULL;
}

void
convertwithdrive(const wchar_t *src, int len, wchar_t *dst)
{
	wchar_t kname[32];
	wchar_t d;
	int pos;
	int cnt = 0;

	memset(dst, 0, 260);
	extractthis(src, kname, &pos);
	kname[pos]='\0';
	pos++;

	d = GetDriveLetter(kname);
	wsprintf(dst, L"%c:\\%ws", d, &src[pos]);
	
}

void SetupDirectory()
{
	wstring			currpath;	
		
	currpath = prog.substr(0, prog.find_last_of('\\'));


	StringCbPrintf(dirpath, MAX_PATH * sizeof(WCHAR), L"%ws\\%ws_analysis", currpath.c_str(), malwarename);
	
	printf("creating directory: %ws \n", dirpath);
	if(!CreateDirectory(dirpath, NULL))
	{
		printf("main level directory could not be created (%d) \n %ws \n", GetLastError(), dirpath);
		if(GetLastError() != 183)
			return ;

	}
}

void
extractfinalname(const wchar_t *src, wchar_t *dst)
{
	const wchar_t *temp ;
	temp = wcsrchr(src, '\\');
	temp++;
	StringCbCopy(dst, 31, temp);
}