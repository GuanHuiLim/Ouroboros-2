#pragma once
#include <functional>
#include <filesystem>
class WindowsUtilities
{
public:
	static bool FileDialogue_Generic(const wchar_t* description, const wchar_t* extension, std::function<void(const std::filesystem::path&)> callback) noexcept;
	static void Windows_Beep_Exclaimation() noexcept;
	static void Windows_Beep_Warn() noexcept;
	static void Windows_Beep_Error() noexcept;
private:

};