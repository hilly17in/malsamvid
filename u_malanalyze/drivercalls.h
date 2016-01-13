# pragma once

#define MALANALYZE_DEVICE_NAME		L"\\\\.\\MAFilterSymLink"


DWORD
StartDriver();

DWORD
StopDriver();

DWORD
InitializeDriver();

DWORD
SendPidToDriver(DWORD);

DWORD
MATerminateProcess(int processid);

int
SendServicePid(int processid);