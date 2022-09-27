/************************************************************************************//*!
\file           FileSystemUtills.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          Supporting functions for file based access 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <filesystem>
class FileSystemUtils
{
public:
	//please do a std::move for this
	static std::vector<std::filesystem::path>  ListOfFilesInDirectory(const std::string& path);
	static std::filesystem::path CreateItem(const std::string& filename, const std::string& ext = {});
	static bool DeleteItem(const std::string& filename);
	static void DuplicateItem(const std::filesystem::path & filename, const std::filesystem::path& targetfolder);
	static size_t CountFiles_Recursively(const std::filesystem::path & filename);
	static size_t CountFiles_NonRecursive(const std::filesystem::path& filename);

private:

};