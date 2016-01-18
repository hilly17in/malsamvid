#include <windows.h>
#include <setupapi.h>
#include <strsafe.h>
#pragma comment(lib, "setupapi.lib")

#define BUFFLEN	MAX_PATH * 2

int main()
{

	//install driver
	InstallHinfSectionW(NULL,NULL,L"DefaultInstall 128 .\\matool.inf",0);

	return 0;
}