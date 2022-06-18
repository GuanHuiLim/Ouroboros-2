#pragma once
#include <windows.h>
#include <winnt.h>
#undef max 
class MiniDumpHelper
{
public:
    /*static HMODULE MiniDumpHelper::LoadDLL(PCWSTR filename);*/

    static long ProcessException(_EXCEPTION_POINTERS* ExceptionInfo);
    static void Init();
    static void DumpMiniDump(PEXCEPTION_POINTERS excpInfo);
private:

};
