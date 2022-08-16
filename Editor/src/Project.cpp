#include "pch.h"
#include "Project.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <SceneManagement/include/SceneManager.h>
#include "App/Editor/UI/Tools/WarningMessage.h"
void Project::LoadProject(std::filesystem::path& config)
{
	std::ifstream ifs(config.string());
	if (ifs.peek() == std::ifstream::traits_type::eof())
	{
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "Config File is not valid!");
		return;
	}

	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);
	auto prj_setting = doc.FindMember("Project Settings");
	
	s_configFile = config;
	s_startingScene = (*prj_setting).value.FindMember("StartScene")->value.GetString();
	s_projectFolder = (*prj_setting).value.FindMember("ProjectFolderPath")->value.GetString();
	s_sceneFolder = (*prj_setting).value.FindMember("SceneFolder")->value.GetString();
	s_scriptcoreDLL = (*prj_setting).value.FindMember("ScriptCoreDLL")->value.GetString();
	s_scriptmodulePath = (*prj_setting).value.FindMember("ScriptModulePath")->value.GetString();
	s_scriptbuildPath = (*prj_setting).value.FindMember("ScriptBuildPath")->value.GetString();
	//scenes to add to scene manager
	
	//end
}

void Project::SaveProject()
{
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Value projectsetting(rapidjson::kObjectType);
	std::string temp = s_projectFolder.string().c_str();
	projectsetting.AddMember("ProjectFolderPath", rapidjson::Value(temp.c_str(),static_cast<rapidjson::SizeType>(temp.size()),doc.GetAllocator()),doc.GetAllocator());
	temp = s_startingScene.string();
	projectsetting.AddMember("StartScene", rapidjson::Value(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator()), doc.GetAllocator());
	temp = s_scriptcoreDLL.string();
	projectsetting.AddMember("ScriptCoreDLL", rapidjson::Value(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator()), doc.GetAllocator());
	temp = s_scriptmodulePath.string();
	projectsetting.AddMember("ScriptModulePath", rapidjson::Value(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator()), doc.GetAllocator());
	temp = s_scriptbuildPath.string();
	projectsetting.AddMember("ScriptBuildPath", rapidjson::Value(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator()), doc.GetAllocator());

	rapidjson::Value scenes(rapidjson::kObjectType);

	//get all scenes


	////
	
	//attatch all members to doc to be serialized
	doc.AddMember("Project Settings", projectsetting, doc.GetAllocator());
	doc.AddMember("Scenes", scenes, doc.GetAllocator());

	std::ofstream ofs(s_configFile);
	if (ofs.good())
	{
		rapidjson::OStreamWrapper osw(ofs);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatDefault);
		writer.SetMaxDecimalPlaces(4);
		doc.Accept(writer);
		ofs.close();
	}
}
