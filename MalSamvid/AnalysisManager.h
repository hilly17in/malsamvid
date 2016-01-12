#pragma once

DWORD WINAPI VBoxThread( LPVOID lpParam );
DWORD WINAPI VMWareThread( LPVOID lpParam );

unsigned int VBoxAnalysis(
  sVMAnalyzer &sDetail
);

unsigned int VMWareAnalysis(
  sVMAnalyzer &sDetail
);

unsigned int CreateSyncObjects();

extern HANDLE ghMutexVMWare;
extern HANDLE ghMutexVbox;

extern HANDLE hThreadArray[]; 