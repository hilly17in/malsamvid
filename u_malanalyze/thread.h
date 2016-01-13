# pragma once

DWORD
WINAPI
CommunicateWithDriver(LPVOID lParam) ;

DWORD
WINAPI
CommunicateWithDriver2(LPVOID lParam) ;

DWORD
WINAPI
WaitForProcessExit(LPVOID lParam) ;

DWORD
WINAPI
NetworkThread(LPVOID lParam);

int GetServicePid() ;

void
WriteCommandLineinReport(unsigned long pid) ;

typedef NTSTATUS (NTAPI *_NtQueryInformationProcess)(
    HANDLE ProcessHandle,
    DWORD ProcessInformationClass,
    PVOID ProcessInformation,
    DWORD ProcessInformationLength,
    PDWORD ReturnLength
    );

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION
{
    LONG ExitStatus;
    PVOID PebBaseAddress;
    ULONG_PTR AffinityMask;
    LONG BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR ParentProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;
