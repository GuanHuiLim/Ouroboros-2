#include "pch.h"
#include "Project.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include "App/Editor/UI/Tools/WarningMessage.h"
void Project::LoadProject(std::filesystem::path& p)
{
	std::ifstream ifs(p.string());
	if (ifs.peek() == std::ifstream::traits_type::eof())
	{
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "Config File is not valid!");
		return;
	}
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);
	auto prj_setting = doc.FindMember("Project Settings");
	s_startingScene = (*prj_setting).value.FindMember("StartScene")->value.GetString();
	s_projectFolder = (*prj_setting).value.FindMember("ProjectFolderPath")->value.GetString();
	s_sceneFolder = (*prj_setting).value.FindMember("SceneFolder")->value.GetString();
	s_scriptcoreDLL = (*prj_setting).value.FindMember("ScriptCoreDLL")->value.GetString();
	s_scriptmodulePath = (*prj_setting).value.FindMember("ScriptModulePath")->value.GetString();
	s_scriptbuildPath = (*prj_setting).value.FindMember("ScriptBuildPath")->value.GetString();
	//scenes to add to scene manager

	//end
}
