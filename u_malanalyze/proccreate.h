# pragma once


DWORD
CreateProcessWrapper(wchar_t *program);

DWORD 
DisplayReport();

VOID RemoveNewFiles();

int findinnotrepeat(wstring str);

VOID DumpQuery(vector<wstring> *datalist, wstring wstr);

VOID DumpFile(vector<data> *datalist, wstring wstr);

VOID DumpFile(vector<hashinfo> *datalist, wstring wstr);

VOID DumpAnalysis() ;

VOID DumpAnalysis_Readfile();

VOID DumpAnalysis_Writefile();

VOID DumpAnalysis_SetValue();

VOID DumpAnalysis_Newfiles();
