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