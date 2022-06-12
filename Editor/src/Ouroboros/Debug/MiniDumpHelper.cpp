#include "pch.h"

#include "MiniDumpHelper.h"
#include "Ouroboros/Core/Application.h"

#include <filesystem>

#include <DbgHelp.h>

#include <windows.h>
#include <winnt.h>
#include <excpt.h>

#ifndef cch
#define ccht(Array, EltType) (sizeof(Array) / sizeof(EltType))
#define cch(Array) ccht(Array, (Array)[0])
#endif


//HMODULE MiniDumpHelper::LoadDLL(PCWSTR filename)
//{
//	WCHAR drive[10] = L"";
//	WCHAR dir[MAX_PATH + 1] = L"";
//	WCHAR file[MAX_PATH + 1] = L"";
//	WCHAR ext[MAX_PATH + 1] = L"";
//	WCHAR path[MAX_PATH + 1] = L"";
//	HMODULE hm;
//
//	// Chop up 'filename' into its elements.
//
//	_wsplitpath_s(filename, drive, cch(drive), dir, cch(dir), file, cch(file), ext, cch(ext));
//
//	// If 'filename' contains no path information, then get the path to our module and 
//	// use it to create a fully qualified path to the module we are loading.  Then load it.
//
//	if (!*drive && !*dir)
//	{
//		// ghinst is the HINSTANCE of this module, initialized in DllMain or WinMain
//		
//		if (GetModuleFileName(NULL, path, MAX_PATH))
//		{
//			_wsplitpath_s(path, drive, cch(drive), dir, cch(dir), NULL, 0, NULL, 0);
//			if (*drive || *dir)
//			{
//				swprintf_s(path, cch(path), L"%s%s%s%s", drive, dir, file, ext);
//				hm = LoadLibrary(path);
//				if (hm)
//					return hm;
//			}
//		}
//	}
//	else
//	{
//		// If we wanted to, we could have LoadDLL also support directories being specified
//		// in 'filename'.  We could pass the path here.  The result is if no path is specified,
//		// the module path is used as above, otherwise the path in 'filename' is specified.
//		// But the standard search logic of LoadLibrary is still avoided.
//
//		/*
//		hm = LoadLibrary(path);
//		if (hm)
//			return hm;
//		*/
//	}
//
//	return 0;
//}

long MiniDumpHelper::ProcessException(_EXCEPTION_POINTERS* ExceptionInfo)
{
    WCHAR file[MAX_PATH + 1] = L"";
    GetModuleFileName(NULL, file, MAX_PATH);
    std::filesystem::path exepath = file;
    std::filesystem::path folder = exepath.parent_path();
    std::filesystem::path mini_dump = (folder.string() + "minidump.dmp");
    HANDLE hFile = CreateFileA(mini_dump.string().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return EXCEPTION_CONTINUE_SEARCH;

    MINIDUMP_EXCEPTION_INFORMATION minidump_info;
    minidump_info.ThreadId = GetCurrentThreadId();
    minidump_info.ExceptionPointers = ExceptionInfo;
    minidump_info.ClientPointers = false;

    bool dump_success = MiniDumpWriteDump(GetCurrentProcess(),
        GetCurrentProcessId(), hFile,
        MINIDUMP_TYPE::MiniDumpWithDataSegs,
        ExceptionInfo ? &minidump_info : nullptr,
        NULL, NULL);
    CloseHandle(hFile);
    if (dump_success == false)
        return EXCEPTION_CONTINUE_SEARCH;

    return EXCEPTION_EXECUTE_HANDLER;
}

void MiniDumpHelper::Init()
{
    SetUnhandledExceptionFilter(ProcessException);
}

void MiniDumpHelper::DumpMiniDump(PEXCEPTION_POINTERS excpInfo)
{
    if (excpInfo == NULL)
    {
        // Generate exception to get proper context in dump
        __try
        {
            RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
        }
        __except (DumpMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }
    else
    {
        ProcessException(excpInfo);
    }
}
