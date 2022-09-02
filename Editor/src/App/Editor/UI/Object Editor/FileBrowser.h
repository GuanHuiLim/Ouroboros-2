#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <imgui/imgui.h>
#include "App/Editor/Events/LoadProjectEvents.h"
class FileBrowser
{
public:
	FileBrowser();
	void SetDirectory(LoadProjectEvent* lpe);
	void Show();
	struct DirectoryInfo
	{
		DirectoryInfo(ImTextureID id, const std::filesystem::path& p, std::function<bool(DirectoryInfo&)> func = {})
			:texid{ id }, name{ p }, end_draw_callback{ func }{};
		ImTextureID texid;
		std::filesystem::path name;
		std::function<bool(DirectoryInfo&)> end_draw_callback;
		bool selected = false;
	};
private:
private://imgui code
	void CreateIconButton(DirectoryInfo& info, ImVec2 size,const float icon_size);
	void CreateIconButton_ListView(DirectoryInfo& info,ImVec2 size, const float icon_size);
	void DirectoryBrowser();
	void DetailedBrowser();
	void DetailBar();
	void OpenPopup();
	void PopupOptions();
	void Find_AndSelect(const std::filesystem::path& p);
private://non imgui
	/*********************************************************************************//*!
	\brief    
	 call this everytime when u reload / change directory
	*//**********************************************************************************/
	void BuildDirectoryList(const std::string& path);
	ImTextureID GetIcon(const std::string& ext);
	void FileBehaviour(DirectoryInfo info);
	void FileBehaviour(const std::filesystem::path& path);
	void RecursiveDirective(const std::filesystem::path& path);
public:
	static bool RenameFilePopup(DirectoryInfo& info);
	static constexpr size_t renamepopupID = 600;
private:
	std::vector<DirectoryInfo> m_directoryList;
	std::vector<DirectoryInfo*>m_selectedList;//possible use for multiselection
	std::vector<std::filesystem::path> m_copybuffer;
	std::filesystem::path m_currentpath = "";
	std::string m_rootpath = "";
	
	std::string m_searchfilter;
	int m_zoomlevel = m_zoomlevelMax;
private://static
	inline static std::unordered_map<std::string, std::string> s_Icons;

	static constexpr int m_zoomlevelMax = 10;
	static constexpr int m_zoomlevelMin = 5;
};