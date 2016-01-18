
#include <windows.h>
#include <setupapi.h>
#include <strsafe.h>
#pragma comment(lib, "setupapi.lib")

int main()
{
	//install driver
	InstallHinfSectionW(NULL,NULL,L"DefaultUnInstall 128 .\\matool.inf",0);
}