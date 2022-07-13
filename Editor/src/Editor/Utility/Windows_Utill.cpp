#include "pch.h"
#include "Windows_Utill.h"

#include <windows.h>
#include <shellapi.h>
#include <shobjidl_core.h>

bool WindowsUtilities::FileDialogue_Generic(const wchar_t* description, const wchar_t* extension, std::function<void(const std::filesystem::path&)> callback) noexcept
{
	bool activated = false;
	std::filesystem::path p;//this folder path
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		//pFileOpen->SetOptions(FOS_PICKFOLDERS);
		const COMDLG_FILTERSPEC filter[] = {
			{description, extension}
		};
		pFileOpen->SetFileTypes(1, filter);
		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);
			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					activated = true;

					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					p = pszFilePath;//this path is e.g (/root/config.exe)
					if (callback)
						callback(p);
					pItem->Release();

				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return activated;
}

void WindowsUtilities::Windows_Beep_Exclaimation() noexcept
{
	MessageBeep(MB_ICONEXCLAMATION);
}

void WindowsUtilities::Windows_Beep_Warn() noexcept
{
	MessageBeep(MB_ICONWARNING);
}

void WindowsUtilities::Windows_Beep_Error() noexcept
{
	MessageBeep(MB_ICONERROR);
}

