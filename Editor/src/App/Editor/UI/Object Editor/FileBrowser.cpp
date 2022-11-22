/************************************************************************************//*!
\file          FileBrowser.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Interact with the files inside
			   Edit Files
			   Open Files
			   Delete Files
			   Create Files
			   Open other UIs


Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "FileBrowser.h"
#include "App/Editor/Events/OpenFileEvent.h"
#include "App/Editor/Events/OpenPromtEvent.h"
#include "App/Editor/Events/FileEventsFunction.h"
#include "App/Editor/Events/LoadProjectEvents.h"
#include "App/Editor/Utility/FileSystemUtills.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "App/Editor/Utility/ImGui_ToggleButton.h"
#include "App/Editor/Utility/ImGuiStylePresets.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/Core/KeyCode.h"
#include "Project.h"

#include "Ouroboros/Animation/AnimationSystem.h"
namespace
{
	bool isSubPath(const std::filesystem::path& base, const std::filesystem::path& destination)
	{
		std::string relative = std::filesystem::relative(destination, base).string();
		return relative.size() == 1 || relative[0] != '.' && relative[1] != '.';
	}
}

FileBrowser::FileBrowser()	
{
	oo::EventManager::Subscribe<OpenFileEvent>(&OpenAnimationEvent);
	oo::EventManager::Subscribe<OpenFileEvent>(&OpenSpriteEvent);
	oo::EventManager::Subscribe<OpenFileEvent>(&OpenFolderEvent);
	oo::EventManager::Subscribe<OpenFileEvent>(&OpenOthersEvent);
	oo::EventManager::Subscribe<FileBrowser,LoadProjectEvent>(this,&FileBrowser::SetDirectory);
}

void FileBrowser::InitAssets()
{
	m_Icons[".png"]			= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/PNGIcon.png");
	m_Icons[".obj"]			= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/OBJFileIcon.png");
	m_Icons[".mp3"]			= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/MP3FileIcon.png");
	m_Icons[".prefab"]		= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/PrefabIcon.png");
	m_Icons[".scn"]			= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/SceneIcon.png");
	m_Icons[".anim"]		= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/AnimationClipIcon.png");
	m_Icons[".controller"]	= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/AnimatorIcon.png");
	m_Icons[".cs"]			= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/CSFileIcon.png");
	m_Icons["generic"]		= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/GenericFileIcon.png");
	m_Icons["folder"]		= ImGuiManager::s_editorAssetManager.GetOrLoadPath("File Icons/FolderIcon.png");
}

inline void FileBrowser::SetDirectory(LoadProjectEvent* lpe)
{
	m_currentpath = lpe->m_projectPath;
	m_rootpath = lpe->m_projectPath;
	FileBehaviour(m_currentpath);
}

void FileBrowser::Show()
{
	if (m_rootpath.empty())
		return;
#ifdef EDITOR_PLATFORM_WINDOWS
	if (isSubPath(Project::GetAssetFolder(), m_currentpath) && ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Compiler"))
		{
			if (ImGui::MenuItem("Convert Current Folder to DDS"))
			{
				std::stringstream ss;
				ss << ".\\converter\\crunch_x64.exe -file ";
				ss << std::filesystem::canonical(m_currentpath);
				ss << " -timestamp -ignoreerrors -fileformat dds -dxt1a -outsamedir";
				LOG_INFO("Running command {0}", ss.str());
				std::system(ss.str().c_str());
				LOG_INFO("Converted assets to dds in {0}", m_currentpath);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
#endif // EDITOR_PLATFORM_WINDOWS
	ImGui::BeginTable("2views", 2,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_Resizable, ImVec2(0, 0), 0);
	ImGui::TableNextColumn();
	DirectoryBrowser();
	ImGui::TableNextColumn();
	DetailedBrowser();
	ImGui::EndTable();
	OpenPopup();
}

void FileBrowser::DetailedBrowser()
{
	ImVec2 region = ImGui::GetContentRegionAvail();
	DetailBar();
	
	constexpr float min_heigh_button = 10.0f; //if width of button is 0 , the height of the button should be min_heigh_button
	constexpr float icon_percentage_size = 0.8f;

	float item_width = { 0 };
	float icon_size = { 0 };
	ImVec2 IconDimensions;
	
	void (FileBrowser:: * functpointer)(DirectoryInfo&, ImVec2, const float);
	if (m_zoomlevel >= m_zoomlevelMin)
	{
		item_width = region.x / m_zoomlevel;
		icon_size = item_width * icon_percentage_size;
		IconDimensions = { item_width,item_width + min_heigh_button };
		functpointer = &FileBrowser::CreateIconButton;
	}
	else
	{
		item_width = ImGui::GetContentRegionAvail().x;
		icon_size = ImGui_StylePresets::image_small.x;
		IconDimensions = { item_width,icon_size };
		functpointer = &FileBrowser::CreateIconButton_ListView;
	}


	ImGui::BeginChild("FileContents", { 0,ImGui::GetContentRegionAvail().y - 10 },false);

	if (ImGui::BeginTable("temp", m_zoomlevel, 0, { 0,0 }, item_width))
	{
		int counter = 0;
		for (auto& directoryInfo : m_directoryList)
		{
			++counter;
			if (m_searchfilter.empty() == false)//search filter
			{
				std::string compare_target = directoryInfo.name.filename().string();
				auto iter = std::search(compare_target.begin(), compare_target.end(), m_searchfilter.begin(), m_searchfilter.end(), [](char a, char b) {return std::toupper(a) == std::toupper(b); });
				if(iter == compare_target.end())
					continue;
			}
			ImGui::PushID(counter);
			ImGui::TableNextColumn();
			(this->*functpointer)(directoryInfo, IconDimensions ,icon_size);
			ImGui::PopID();
			if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
			{
				FileBehaviour(directoryInfo);
				break;
			}
			if(directoryInfo.end_draw_callback)
				directoryInfo.end_draw_callback(directoryInfo);
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void FileBrowser::DetailBar()
{
	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	float arrowbutton_offset = 0;
	ImGui::BeginGroup();

	if (m_currentpath > m_rootpath)
	{
		if (ImGui::ArrowButton("Button Back", ImGuiDir_::ImGuiDir_Left))
		{
			FileBehaviour(m_currentpath.parent_path());
		}
	}
	else
	{
		ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGui::ArrowButton("Button Back", ImGuiDir_::ImGuiDir_Left);
		ImGui::PopStyleColor();
		ImGui::PopItemFlag();
	}

	ImGui::SameLine();
	{
		arrowbutton_offset = ImGui::GetCursorPosX();
		
		ImGui::PushItemWidth(contentRegion.x *0.7f);
		ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
		std::string currentpath_str = m_currentpath.make_preferred().string();
		ImGui::InputText("##NamePath", &currentpath_str, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
		ImGui::PopItemFlag();
		ImGui::PopItemWidth();
	}
	{
		ImGui::SameLine();
		bool listview = m_zoomlevel < m_zoomlevelMin;
		ImGui::PushItemWidth(-30.0f);
		if (ImGui::SliderInt("##SizeControl", &m_zoomlevel, m_zoomlevelMin - 1, m_zoomlevelMax, (listview) ? "List View" : "%d Items", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp))
		{
			if (m_zoomlevel < m_zoomlevelMin)
				m_zoomlevel = 1;//list view
		}
		ImGui::PopItemWidth();
	}
	//{//sort by function
	//	ImGui::SameLine();
	//	if (ImGui::ImageButton(s_Icons[".controller"],ImGuiManager::image_small))
	//	{

	//	}
	//}
	//ImGui::Separator();
	{

		ImGui::SetCursorPosX(arrowbutton_offset);
		ImGui::PushItemWidth(contentRegion.x * 0.25f);
		ImGui::InputText("Filter", &m_searchfilter, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::PopItemWidth();
	}
	ImGui::EndGroup();
}

void FileBrowser::OpenPopup()
{
	if (ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Right) && ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_ChildWindows))
	{
		ImGui::OpenPopup("Options");
	}
	if (ImGui::BeginPopup("Options", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
	{
		PopupOptions();
		ImGui::EndPopup();
	}
}

void FileBrowser::PopupOptions()
{
	if (ImGui::BeginMenu("Create"))
	{
		if (ImGui::MenuItem("Create Folder"))
		{
			std::string fileTemp = (m_currentpath.string() + "/Newfolder");
			std::filesystem::path result_path = FileSystemUtils::CreateItem(fileTemp, {});
			FileBehaviour(result_path.parent_path());
			Find_AndSelect(result_path);
		}
		if (ImGui::MenuItem("Create Scene"))
		{
			std::string fileTemp = (Project::GetSceneFolder().string() + "/Scene");
			std::filesystem::path result_path = FileSystemUtils::CreateItem(fileTemp, {".scn"});
			FileBehaviour(result_path.parent_path());
			Find_AndSelect(result_path);
		}
		if (ImGui::MenuItem("Create Script"))
		{
			/*std::string fileTemp = (Project::GetScriptFolder().string() + "/NewScript");
			std::filesystem::path result_path = FileSystemUtils::CreateItem(fileTemp, {".cs"});
			FileBehaviour(result_path.parent_path());
			Find_AndSelect(result_path);*/
		}
		if (ImGui::MenuItem("Create Animation Tree"))
		{
			oo::Anim::AnimationSystem::CreateAnimationTree("New Animation Tree");
		}
		if (ImGui::MenuItem("Create Animation"))
		{
			//oo::Anim::AnimationSystem::crea
		}
		ImGui::EndMenu();
	}
	ImGui::Separator();
	if (ImGui::MenuItem("Delete", 0, false, !m_selectedList.empty()))
	{
		for (auto& items : m_selectedList)
		{
			FileSystemUtils::DeleteItem(items->name.string());
		}
		FileBehaviour(m_currentpath.string());
	}
#ifdef EDITOR_PLATFORM_WINDOWS
	ImGui::Separator();
	if (ImGui::MenuItem("Open File Location"))
	{
		ShellExecuteA(NULL, "explore", m_currentpath.string().c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
#endif // EDITOR_PLATFORM_WINDOWS
	ImGui::Separator();
	if (ImGui::MenuItem("Copy"))
	{
		m_copybuffer.clear();
		for (auto* item : m_selectedList)
			m_copybuffer.push_back(item->name);
	}
	if (ImGui::MenuItem("Paste"))
	{
		for (auto& item : m_copybuffer)
			FileSystemUtils::DuplicateItem(item,m_currentpath);
		FileBehaviour(m_currentpath.string());
		
	}
	if (ImGui::MenuItem("Rename",nullptr,nullptr,m_selectedList.size() == 1))
	{
		ImGui::OpenPopup(renamepopupID);
		m_selectedList.back()->end_draw_callback = RenameFilePopup;
	}
}

void FileBrowser::Find_AndSelect(const std::filesystem::path& p)
{
	m_selectedList.clear();
	for (auto& item : m_directoryList)
	{
		if (item.name == p)
		{
			item.selected = true;
			m_selectedList.emplace_back(&item);
		}
	}
}

void FileBrowser::DirectoryBrowser()
{
	//recurse through the whole folder and only look for folders
	ImVec2 size = ImGui::GetContentRegionAvail();
	ImGui::BeginGroup();

	ImGui::BeginGroup();
	//generic
	ImGui::Image(m_Icons["generic"].GetData<ImTextureID>(), ImGui_StylePresets::image_small);
	ImGui::SameLine();
	if (ImGui::Selectable("Assets"))
		FileBehaviour(Project::GetAssetFolder());
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Image(m_Icons[".prefab"].GetData<ImTextureID>(), ImGui_StylePresets::image_small);
	ImGui::SameLine();
	if (ImGui::Selectable("Prefab"))
		FileBehaviour(Project::GetPrefabFolder());
	ImGui::EndGroup();

	ImGui::BeginGroup();
	ImGui::Image(m_Icons[".scn"].GetData<ImTextureID>(), ImGui_StylePresets::image_small);
	ImGui::SameLine();
	if (ImGui::Selectable("Scene"))
		FileBehaviour(Project::GetSceneFolder());
	ImGui::EndGroup();

	//ImGui::BeginGroup();
	//ImGui::Image(ImGuiManager::s_EditorIcons["CSFileIcon"], ImGui_StylePresets::image_small);
	//ImGui::SameLine();
	//if (ImGui::Selectable("Script"))
	//	FileBehaviour(Project::GetScriptFolder());
	//ImGui::EndGroup();

	ImGui::EndGroup();

	ImGui::Separator();

	ImGui::BeginChild("directory", {0,ImGui::GetContentRegionAvail().y - 10});
	RecursiveDirective(m_rootpath);
	ImGui::EndChild();
}

void FileBrowser::CreateIconButton(DirectoryInfo& info, ImVec2 size, const float icon_size)
{
	ImVec2 orignal_cursorPos = ImGui::GetCursorPos();
	ImVec2 icon_dimensions = { icon_size,icon_size };
	ImGui::PushID(info.name.c_str());

	ImGui::BeginGroup();
	ImGui::Image(info.texid, icon_dimensions);
	ImVec2 text_len = ImGui::CalcTextSize(info.name.filename().string().c_str());
	if (text_len.x <= icon_size)
	{
		ImGui::Dummy({ 0,0 });
		ImGui::SameLine((icon_size - text_len.x) * 0.5f);
	}
	if (info.selected)
	{
		ImGui::TextWrapped(info.name.filename().string().c_str());
	}
	else
	{
		ImGui::Text(info.name.filename().string().c_str());
	}
	ImGui::Dummy({ icon_size,size.y - icon_size });
	ImGui::EndGroup();

	ImGui::SetCursorPos(orignal_cursorPos);
	bool selected = ImGui::Selectable("##IconSelection", &info.selected, ImGuiSelectableFlags_AllowDoubleClick, icon_dimensions);
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAutoExpirePayload) && info.name.has_extension())
	{
		ImGui::SetDragDropPayload(info.name.extension().string().c_str(), &info.name, sizeof(std::filesystem::path));
		ImGui::Text("Dragging type : %s", info.name.extension().string().c_str());
		ImGui::EndDragDropSource();
	}
	if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && m_selectedList.empty())
	{
		info.selected = true;
		selected = true;
	}
	if (selected)
	{
		if (info.selected == true)
		{
			if (ImGui::IsKeyDown(static_cast<int>(oo::input::KeyCode::LCTRL)))
			{
				m_selectedList.emplace_back(&info);
			}
			else
			{
				for (auto* selected_items : m_selectedList)
					selected_items->selected = false;
				m_selectedList.clear();
				m_selectedList.emplace_back(&info);
			}
		}
		else
		{
			if(ImGui::IsKeyDown(static_cast<int>(oo::input::KeyCode::LCTRL)))
			{
				auto iter = std::find(m_selectedList.begin(), m_selectedList.end(), &info);
				m_selectedList.erase(iter);
			}
			else
			{
				for (auto* selected_items : m_selectedList)
					selected_items->selected = false;
				m_selectedList.clear();
			}
		}
	}
	ImGui::PopID();
}

void FileBrowser::CreateIconButton_ListView(DirectoryInfo& info,ImVec2 size, const float icon_size)
{
	ImVec2 cursorPosition = ImGui::GetCursorPos();
	ImVec2 icon_dimensions = { icon_size,icon_size };
	ImGui::PushID(info.name.c_str());
	ImGui::Image(info.texid, icon_dimensions);
	ImGui::SameLine();
	ImGui::Text(info.name.filename().string().c_str());
	ImGui::SetCursorPos(cursorPosition);
	if (ImGui::Selectable("##selectable", &info.selected, ImGuiSelectableFlags_AllowDoubleClick, size))
	{
		if (info.selected == true)
		{
			if (ImGui::IsKeyDown(static_cast<int>(oo::input::KeyCode::LCTRL)))
			{
				m_selectedList.emplace_back(&info);
			}
			else
			{
				for (auto* selected_items : m_selectedList)
					selected_items->selected = false;
				m_selectedList.clear();
				m_selectedList.emplace_back(&info);
			}
		}
		else
		{
			if (ImGui::IsKeyDown(static_cast<int>(oo::input::KeyCode::LCTRL)))
			{
				auto iter = std::find(m_selectedList.begin(), m_selectedList.end(), &info);
				m_selectedList.erase(iter);
			}
			else
			{
				for (auto* selected_items : m_selectedList)
					selected_items->selected = false;
				m_selectedList.clear();
			}
		}
	}
	ImGui::PopID();
}



void FileBrowser::BuildDirectoryList(const std::string& path)
{
	auto list = FileSystemUtils::ListOfFilesInDirectory(path);
	m_currentpath = path;
	m_directoryList.reserve(list.size());
	m_directoryList.clear();//clear all directory
	m_selectedList.clear();//clear all selection
	
	//TODO: find a beter soln for this
	//load the assets if this triggers early
	if (m_Icons.empty())
		InitAssets();
	
	for (auto& directory : list)
	{
		if (directory.extension() == oo::Asset::EXT_META)
		{
			continue;
		}
		m_directoryList.emplace_back(DirectoryInfo{ GetIcon(directory.extension().string()),directory});
	}
}

ImTextureID FileBrowser::GetIcon(const std::string& ext)
{
	//folder
	if (ext.empty())
		return m_Icons["folder"].GetData<ImTextureID>();

	auto iter = m_Icons.find(ext);
	//generic
	if (iter == m_Icons.end())
		return m_Icons["generic"].GetData<ImTextureID>();

	return iter->second.GetData<ImTextureID>();
}

void FileBrowser::FileBehaviour(DirectoryInfo info)
{
	std::string ext = info.name.extension().string();
	if(ext.empty())
		BuildDirectoryList(info.name.string());

	OpenFileEvent e(info.name);
	OpenPromptEvent<OpenFileEvent> ope(e, 0);
	oo::EventManager::Broadcast(&ope);
}

void FileBrowser::FileBehaviour(const std::filesystem::path& path)
{
	std::string ext = path.extension().string();
	if (ext.empty())
		BuildDirectoryList(path.string());

	OpenFileEvent e = path;
	OpenPromptEvent<OpenFileEvent> ope(e,0);
	oo::EventManager::Broadcast(&ope);
}

void FileBrowser::RecursiveDirective(const std::filesystem::path& path)
{
	ImGuiTreeNodeFlags flag = 0;


	bool enable = false;
	for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_directory())//if item is a folder use the folder img
		{
			continue;
		}
		//generic
		ImGui::Image(m_Icons["generic"].GetData<ImTextureID>(), {15,15});
		flag = ImGuiTreeNodeFlags_OpenOnArrow;

		ImGui::SameLine();

		//check if relative
		if (std::filesystem::equivalent(entry.path(),m_currentpath))
		{
			flag |= ImGuiTreeNodeFlags_Selected;
		}


		enable = ImGui::TreeNodeEx(entry.path().filename().string().c_str(), flag);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right) || (ImGui::IsItemClicked(ImGuiMouseButton_Left)))
		{
			if (entry.is_directory())//for the previde module
			{
				m_currentpath = entry.path();
				FileBehaviour(entry.path());
			}
		}
		

		if (enable && entry.is_directory())
		{
			ImGui::PushID(entry.path().filename().u8string().c_str());
			
			RecursiveDirective(entry.path().string());
			
			ImGui::PopID();
			ImGui::TreePop();//pop the tree nodes
		}

	}
}

bool FileBrowser::RenameFilePopup(DirectoryInfo& info)
{
	static bool cleared = false;
	static std::string renameBuffer = "";
	bool completed = false;
	ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
	bool popup = ImGui::BeginPopupEx(renamepopupID, ImGuiWindowFlags_NoDecoration| ImGuiWindowFlags_AlwaysAutoResize);
	if (popup)
	{
		if (cleared == false)
		{
			cleared = true;
			renameBuffer = info.name.stem().string();
		}
		else if (ImGui::InputText("##rename",&renameBuffer,ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue| ImGuiInputTextFlags_AutoSelectAll))
		{
			auto oldAssetPath = info.name;
			auto oldMetaPath = oldAssetPath;
			oldMetaPath += oo::Asset::EXT_META;

			renameBuffer += info.name.extension().string();
			std::string newAssetName = renameBuffer;
			std::string newMetaName = renameBuffer + oo::Asset::EXT_META;

			auto newAssetPath = info.name.replace_filename(newAssetName);
			auto newMetaPath = newAssetPath;
			newMetaPath += oo::Asset::EXT_META;

			std::filesystem::rename(oldAssetPath, newAssetPath);
			std::filesystem::rename(oldMetaPath, newMetaPath);
			ImGui::CloseCurrentPopup();

			Project::GetAssetManager()->Scan();
		}
		ImGui::EndPopup();
	}
	else
	{
		cleared = false;
		renameBuffer = "";
		info.end_draw_callback = nullptr;
		completed = true;
	}
	return completed;
}


