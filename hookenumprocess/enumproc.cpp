//////////////////////////////////////////////////////////////////////////////
//
//  Detours Test Program (simple.cpp of simple.dll)
//
//  Microsoft Research Detours Package, Version 3.0.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  This DLL will detour the Windows SleepEx API so that TimedSleep function
//  gets called instead.  TimedSleepEx records the before and after times, and
//  calls the real SleepEx API through the TrueSleepEx function pointer.
//
#include <stdio.h>
#include <windows.h>
#include <psapi.h>


#include <iostream>
#include <vector>
using namespace std;

#include "detours.h"
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "winmm.lib")

LARGE_INTEGER freq;

typedef struct _Record
{	
	LPVOID				HeapAddress;
	HANDLE				HeapHandle;
	SIZE_T				HeapSize;
	LARGE_INTEGER		HeapStart;
	LARGE_INTEGER		HeapEnd;
	double				HeapLife;
	int					cnt;
	char				isFree;
} Record;

//Record vRecord[9999];


int heapcnt=0;
//
//void UpdateDifference(Record &temp)
//{
//	temp.HeapLife = temp.HeapEnd - temp.HeapStart;
//}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
//FILE *fp_heap ;
FILE *fp_all;

//static BOOL (WINAPI * TrueHeapFree)(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) = HeapFree;
//static BOOL (WINAPI * TrueHeapDestroy)(HANDLE hHeap) = HeapDestroy;


//#if _MSC_VER < 1300
//LPVOID (WINAPI *
//        TrueHeapAlloc)(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes)
//    = HeapAlloc;
//#else
//LPVOID (WINAPI *
//        TrueHeapAlloc)(HANDLE hHeap, DWORD dwFlags, DWORD_PTR dwBytes)
//    = HeapAlloc;
//#endif

static BOOL (WINAPI * TrueEnumProcesses)(DWORD *pProcessIds, DWORD cb, DWORD *pBytesReturned) = EnumProcesses;
static HANDLE (WINAPI * TrueOpenProcess)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId) = OpenProcess;
static BOOL (WINAPI * TrueReadProcessMemory)(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead) = ReadProcessMemory;
static BOOL (WINAPI * TrueWriteProcessMemory)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesWritten) = WriteProcessMemory;
static BOOL (WINAPI * TrueCreateProcess)(LPCTSTR lpApplicationName,
										 LPTSTR lpCommandLine,
										 LPSECURITY_ATTRIBUTES lpProcessAttributes,
										 LPSECURITY_ATTRIBUTES lpThreadAttributes,
										 BOOL bInheritHandles,
										 DWORD dwCreationFlags,
										 LPVOID lpEnvironment,
										 LPCTSTR lpCurrentDirectory,
										 LPSTARTUPINFO lpStartupInfo,
										 LPPROCESS_INFORMATION lpProcessInformation
										 ) = CreateProcess;
 
BOOL __declspec(dllexport) WINAPI MPCreateProcess(LPCTSTR lpApplicationName,
										 LPTSTR lpCommandLine,
										 LPSECURITY_ATTRIBUTES lpProcessAttributes,
										 LPSECURITY_ATTRIBUTES lpThreadAttributes,
										 BOOL bInheritHandles,
										 DWORD dwCreationFlags,
										 LPVOID lpEnvironment,
										 LPCTSTR lpCurrentDirectory,
										 LPSTARTUPINFO lpStartupInfo,
										 LPPROCESS_INFORMATION lpProcessInformation
										 )
{
	BOOL	ret;
	char	DLLPath[MAX_PATH];
	
	//ret = TrueCreateProcess(lpApplicationName,
	//						lpCommandLine,
	//						lpProcessAttributes,
	//						lpThreadAttributes,
	//						bInheritHandles,
	//						dwCreationFlags,
	//						lpEnvironment,
	//						lpCurrentDirectory,
	//						lpStartupInfo,
	//						lpProcessInformation
	//					);
	
	GetModuleFileNameA((HINSTANCE)&__ImageBase, DLLPath, MAX_PATH);
	//fprintf(fp_all, "DLL Path is %s \n", DLLPath);
	//fprintf(fp_all, "args of detourcreateprocess %s is %d \n", lpApplicationName, lpCommandLine);

	ret = DetourCreateProcessWithDllEx(lpApplicationName, 
								 lpCommandLine,
								 lpProcessAttributes,
								 lpThreadAttributes,
								 bInheritHandles,
								 dwCreationFlags,
								 lpEnvironment,
								 lpCurrentDirectory,
								 lpStartupInfo,
								 lpProcessInformation,
								 DLLPath,
								 TrueCreateProcess);

                          
	//fprintf(fp_all, "ret of detourcreateprocess %s-%s is %d \n", lpApplicationName, lpCommandLine, ret);
	
	return ret;
} 



BOOL __declspec(dllexport) WINAPI MPEnumProcesses(DWORD *pProcessIds, DWORD cb, DWORD *pBytesReturned)
{
	BOOL ret;
	
	ret = TrueEnumProcesses(pProcessIds, cb, pBytesReturned);
	
	// send info to thread in u_malanalyze
	
	fprintf(fp_all, "Enumerates-Processes \n");

    return ret;
}

HANDLE __declspec(dllexport) WINAPI MPOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
	HANDLE	hProcess;
	WCHAR	ImageFileName[1024];
	DWORD	dwCurrId;

	dwCurrId = GetCurrentProcessId();
	hProcess = TrueOpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
	
	if(dwCurrId == dwProcessId)		// this is insignificant
		return hProcess;

	if(GetProcessImageFileName(hProcess, ImageFileName, 1023))
			fprintf(fp_all, "%d, Opening Process (%d), %ws \n", dwCurrId, dwProcessId, ImageFileName);
		else
			fprintf(fp_all, "%d, Opening Process (%d)\n", dwCurrId, dwProcessId);

	return hProcess;
}

BOOL __declspec(dllexport) WINAPI MPReadProcessMemory(HANDLE hProcess, 
													  LPCVOID lpBaseAddress, 
													  LPVOID lpBuffer, 
													  SIZE_T nSize, 
													  SIZE_T *lpNumberOfBytesRead)
{
	BOOL	ret;
	DWORD	dwProcessId;
	DWORD	dwCurrId;
	WCHAR	ImageFileName[1024];

	ret = TrueReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
	dwProcessId = GetProcessId(hProcess);
	dwCurrId = GetCurrentProcessId();

	if(dwCurrId == dwProcessId){
		fprintf(fp_all, "read-own-process-memory \n");
		return ret;
	}

	if(GetProcessImageFileName(hProcess, ImageFileName, 1023))
		fprintf(fp_all, "read-process-memory, %d,(%d), %ws \n", dwCurrId, dwProcessId, ImageFileName);
	else
		fprintf(fp_all, "read-process-memory, %d, %d \n", dwCurrId, dwProcessId);

	return ret;
}

BOOL __declspec(dllexport) WINAPI  MPWriteProcessMemory(HANDLE hProcess, 
														LPVOID lpBaseAddress, 
														LPCVOID lpBuffer, 
														SIZE_T nSize, 
														SIZE_T *lpNumberOfBytesWritten)
{
	BOOL	ret ;
	DWORD	dwProcessId;
	DWORD	dwCurrId;
	WCHAR	ImageFileName[1024];

	ret = TrueWriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);

	dwProcessId = GetProcessId(hProcess);
	dwCurrId = GetCurrentProcessId();

	if(dwCurrId == dwProcessId){
		fprintf(fp_all, "write-own-process-memory \n");
		return ret;
	}

	if(GetProcessImageFileName(hProcess, ImageFileName, 1023))
		fprintf(fp_all, "write-process-memory, %d, (%d), %ws \n", dwCurrId, dwProcessId, ImageFileName);
	else
		fprintf(fp_all, "write-process-memory, %d, (%d)\n", dwCurrId, dwProcessId);

	return ret;
}

//BOOL _declspec(dllexport) MPHeapDestroy(HANDLE hHeap)
//{
//	int counter=0;
//
//	while(counter <= heapcnt)
//	{
//		if(  (vRecord[counter].HeapHandle == hHeap) && (vRecord[counter].isFree == 0) ) 
//		{
//			QueryPerformanceCounter(&vRecord[counter].HeapEnd);
//			double delta_milliseconds = (double)(vRecord[counter].HeapEnd.QuadPart - vRecord[counter].HeapStart.QuadPart) / freq.QuadPart * 1000.0;
//			//fprintf(fp_heap, "%0.16p, %0.8d, %0.16f, d\n", vRecord[counter].HeapAddress, vRecord[counter].HeapSize, delta_milliseconds);
//			vRecord[counter].isFree = 1;
//		}
//		
//		++counter;
//	}
//
//	return TrueHeapDestroy(hHeap);
//}

//BOOL __declspec(dllexport) WINAPI MPHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
//{
//	int counter=0;
//	while(counter <= heapcnt)
//	{
//		if ( (lpMem!=0) && (vRecord[counter].HeapAddress == lpMem) && (vRecord[counter].isFree == 0) )
//		{
//			QueryPerformanceCounter(&vRecord[counter].HeapEnd);
//		
//			double delta_milliseconds = (double)(vRecord[counter].HeapEnd.QuadPart - vRecord[counter].HeapStart.QuadPart) / freq.QuadPart * 1000.0;
//
//			//fprintf(fp_heap, "%0.16p, %0.8d, %0.16f, f\n", lpMem, vRecord[counter].HeapSize, delta_milliseconds);
//
//			vRecord[counter].isFree = 1;
//
//			break;
//		}
//	
//		++counter;
//	} // while
//
//	// print to CSV file.
//
//
//	return TrueHeapFree(hHeap, dwFlags, lpMem);
//}

//#if _MSC_VER < 1300
//LPVOID __declspec(dllexport) WINAPI MPHeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes)
//#else
//LPVOID __declspec(dllexport) WINAPI MPHeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD_PTR dwBytes)
//#endif
//{	
//	LPVOID rv=0;
//	
//	rv = TrueHeapAlloc(hHeap, dwFlags, dwBytes);
//
//	if( (rv !=0) && ( dwBytes!=0 ) && (heapcnt<9999) )
//	{
//		vRecord[heapcnt].HeapHandle = hHeap;
//		vRecord[heapcnt].HeapSize = dwBytes;
//	
//		QueryPerformanceCounter(&vRecord[heapcnt].HeapStart);
//		
//		vRecord[heapcnt].HeapAddress = rv;
//		vRecord[heapcnt].cnt = heapcnt;
//		vRecord[heapcnt].isFree = 0;
//		++heapcnt;
//	}
//	
//	return rv;
//}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    LONG error;
    (void)hinst;
    (void)reserved;

	QueryPerformanceFrequency(&freq);
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
		fp_all=fopen("ignore.txt", "w");
		//fp_heap=fopen("heap.csv", "a+");

        DetourRestoreAfterWith();

        //printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"" Starting.\n");
        fflush(stdout);

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
  
		//DetourAttach(&(PVOID&)TrueHeapAlloc, MPHeapAlloc);
		//DetourAttach(&(PVOID&)TrueHeapFree, MPHeapFree);
		
		DetourAttach(&(PVOID&)TrueCreateProcess, MPCreateProcess);
		DetourAttach(&(PVOID&)TrueEnumProcesses, MPEnumProcesses);
		DetourAttach(&(PVOID&)TrueOpenProcess, MPOpenProcess);
		DetourAttach(&(PVOID&)TrueReadProcessMemory, MPReadProcessMemory);
		DetourAttach(&(PVOID&)TrueWriteProcessMemory, MPWriteProcessMemory);

        error = DetourTransactionCommit();

        if (error == NO_ERROR) {
            /*printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
                   " Detoured EnumProcesses().\n");*/
        }
        else {
            /*printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
                   " Error detouring EnumProcesses(): %d\n", error);*/
        }
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
	
		//DetourDetach(&(PVOID&)TrueHeapAlloc, MPHeapAlloc);
		//DetourDetach(&(PVOID&)TrueHeapFree, MPHeapFree);
		//
		DetourDetach(&(PVOID&)TrueCreateProcess, MPCreateProcess);
		DetourDetach(&(PVOID&)TrueEnumProcesses, MPEnumProcesses);
		DetourDetach(&(PVOID&)TrueOpenProcess, MPOpenProcess);
		DetourDetach(&(PVOID&)TrueReadProcessMemory, MPReadProcessMemory);
		DetourDetach(&(PVOID&)TrueWriteProcessMemory, MPWriteProcessMemory);

        error = DetourTransactionCommit();

		//fclose(fp_heap);
		fclose(fp_all);
        //printf("simple" DETOURS_STRINGIFY(DETOURS_BITS) ".dll:"
        //       " Removed EnumProcesses() (result=%d), \n", error);
        fflush(stdout);
    }
    return TRUE;
}

//
///////////////////////////////////////////////////////////////// End of File.
