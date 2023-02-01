/************************************************************************************//*!
\file           Windows_Utill.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          contains helper functions for windows actions 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
	static bool FileDialogue_Folder(std::function<void(const std::filesystem::path&)> callback);
private:

};