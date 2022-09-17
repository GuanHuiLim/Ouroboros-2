#include "pch.h"
#include "Project.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <SceneManagement/include/SceneManager.h>
#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Events/LoadProjectEvents.h"


#include "Ouroboros/Scene/RuntimeController.h"
#include "App/Editor/Utility/ImGuiManager.h"

#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/Scripting/ScriptManager.h"
#include "Ouroboros/Input/InputManager.h"

void Project::LoadProject(std::filesystem::path& config)
{
	static bool run_once = []() {	oo::EventManager::Subscribe<CloseProjectEvent>([](CloseProjectEvent*) {SaveProject(); }); return true; }();

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
	s_projectFolder = s_configFile.parent_path();
	s_assetFolder = (*prj_setting).value.FindMember("AssetFolder")->value.GetString();
	s_prefabFolder = (*prj_setting).value.FindMember("PrefabFolder")->value.GetString();
	s_startingScene = (*prj_setting).value.FindMember("StartScene")->value.GetString();
	s_sceneFolder = (*prj_setting).value.FindMember("SceneFolder")->value.GetString();
	s_scriptcoreDLL = (*prj_setting).value.FindMember("ScriptCoreDLL")->value.GetString();
	s_scriptmodulePath = (*prj_setting).value.FindMember("ScriptModulePath")->value.GetString();
	s_scriptbuildPath = (*prj_setting).value.FindMember("ScriptBuildPath")->value.GetString();

    // create/load scripting stuff
    UpdateScriptingFiles();
    oo::ScriptManager::LoadProject(GetScriptBuildPath().string(), GetScriptModulePath().string());

    // load input manager
    oo::InputManager::LoadDefault();

	//scenes to add to scene manager
	oo::RuntimeController::container_type m_loadpaths;
	auto scenes_settings = doc.FindMember("Scenes");
	for (auto iter = scenes_settings->value.MemberBegin(); iter != scenes_settings->value.MemberEnd(); ++iter)
	{
		m_loadpaths.emplace_back(oo::SceneInfo{ iter->name.GetString() , s_projectFolder.string() + s_sceneFolder.string() + iter->value.GetString() });
	}
	LoadProjectEvent lpe(std::move(s_projectFolder.string() + s_sceneFolder.string() + s_startingScene.string()), std::move(m_loadpaths),std::move(s_projectFolder.string()));
	oo::EventManager::Broadcast(&lpe);
	//end
	ifs.close();

	//load assets here
	std::filesystem::path hard_assetfolderpath = GetAssetFolder();
	s_AssetManager = std::make_shared<oo::AssetManager>(hard_assetfolderpath);
	s_AssetManager->LoadDirectoryAsync(hard_assetfolderpath, true);
}

void Project::SaveProject()
{
	std::ifstream ifs(s_configFile.string());
	if (ifs.peek() == std::ifstream::traits_type::eof())
	{
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "Config File is not valid!");
		return;
	}

	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);
	auto prj_setting = doc.FindMember("Project Settings");
	std::filesystem::path p = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->GetFilePath();
	std::filesystem::path relative = p.lexically_relative(GetSceneFolder());
	prj_setting->value.FindMember("StartScene")->value.SetString(relative.string().c_str(), static_cast<rapidjson::SizeType>(relative.string().size()), doc.GetAllocator());

	auto scenes  = doc.FindMember("Scenes");
	scenes->value.RemoveAllMembers();
	auto size = scenes->value.MemberCount();
	auto* runtimecontroller = ImGuiManager::s_runtime_controller;
	auto loadpaths = runtimecontroller->GetLoadPaths();
	for(auto scene_info : loadpaths)
	{
		rapidjson::Value name(scene_info.SceneName.c_str(),doc.GetAllocator());
		std::filesystem::path scene_loadpath = std::filesystem::relative(scene_info.LoadPath, GetSceneFolder());
		rapidjson::Value data(scene_loadpath.string().c_str(),doc.GetAllocator());
		scenes->value.AddMember(name, data, doc.GetAllocator());
	}
		//write your scenes
	//doc.AddMember("Scenes", scenes,doc.GetAllocator());

	//get all scenes


	////
	
	//attatch all members to doc to be serialized
	//doc.AddMember("Project Settings", projectsetting, doc.GetAllocator());
	//doc.AddMember("Scenes", scenes, doc.GetAllocator());

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
	ifs.close();
}

void Project::UpdateScriptingFiles()
{
    // Update ScriptCore.dll
    std::filesystem::path currDllPath = GetScriptCoreDLL();
    if (!std::filesystem::exists(currDllPath.parent_path()))
    {
        std::filesystem::create_directory(currDllPath.parent_path());
    }

    std::filesystem::path sourceDllPath = "ScriptCore.dll";
    if (!std::filesystem::exists(sourceDllPath))
    {
        sourceDllPath = "dlls/ScriptCore.dll";
    }
    if (std::filesystem::exists(currDllPath))
    {
        // If not the same ScriptCore.dll, update it to the current version
        std::filesystem::file_time_type currTime = std::filesystem::last_write_time(currDllPath);
        std::filesystem::file_time_type sourceTime = std::filesystem::last_write_time(sourceDllPath);
        if (sourceTime != currTime)
        {
            std::filesystem::copy_file(sourceDllPath, currDllPath, std::filesystem::copy_options::overwrite_existing);
            LOG_TRACE("ScriptCore.dll updated");
        }
    }
    else
    {
        // no ScriptCore.dll, add it
        std::filesystem::copy_file(sourceDllPath, currDllPath, std::filesystem::copy_options::overwrite_existing);
        LOG_TRACE("ScriptCore.dll added");
    }

    // csproj already exists, do nothing
    if (std::filesystem::exists(GetScriptModulePath()))
        return;

    // csproj doesn't exist, create csproj and sln

    // generate VS C# project
    std::ofstream vsProj(GetScriptModulePath());
    if (!vsProj)
        return;
    vsProj << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
    vsProj << "<Project ToolsVersion=\"15.0\" DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << std::endl;
    vsProj << "  <Import Project=\"$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props\" Condition=\"Exists('$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props')\" />" << std::endl;
    vsProj << "  <PropertyGroup>" << std::endl;
    vsProj << "    <Configuration Condition=\" '$(Configuration)' == '' \">Debug OpenGL</Configuration>" << std::endl;
    vsProj << "    <Platform Condition=\" '$(Platform)' == '' \">x64</Platform>" << std::endl;
    vsProj << "    <ProjectGuid>{}</ProjectGuid>" << std::endl;
    vsProj << "    <OutputType>Library</OutputType>" << std::endl;
    vsProj << "    <AppDesignerFolder>Properties</AppDesignerFolder>" << std::endl;
    vsProj << "    <RootNamespace>Scripting</RootNamespace>" << std::endl;
    vsProj << "    <AssemblyName>Scripting</AssemblyName>" << std::endl;
    vsProj << "    <TargetFrameworkVersion>v4.7.2</TargetFrameworkVersion>" << std::endl;
    vsProj << "    <FileAlignment>512</FileAlignment>" << std::endl;
    vsProj << "    <AutoGenerateBindingRedirects>true</AutoGenerateBindingRedirects>" << std::endl;
    vsProj << "  </PropertyGroup>" << std::endl;
    vsProj << "    <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Debug OpenGL|x64' \">" << std::endl;
    vsProj << "    <PlatformTarget>x64</PlatformTarget>" << std::endl;
    vsProj << "    <DebugSymbols>true</DebugSymbols>" << std::endl;
    vsProj << "    <DebugType>pdbonly</DebugType>" << std::endl;
    vsProj << "    <Optimize>false</Optimize>" << std::endl;
    vsProj << "    <OutputPath>bin\\Debug\\</OutputPath>" << std::endl;
    vsProj << "    <DefineConstants></DefineConstants>" << std::endl;
    vsProj << "    <ErrorReport>prompt</ErrorReport>" << std::endl;
    vsProj << "    <WarningLevel>4</WarningLevel>" << std::endl;
    vsProj << "  </PropertyGroup>" << std::endl;
    vsProj << "  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Production OpenGL|x64' \">" << std::endl;
    vsProj << "    <DebugType>pdbonly</DebugType>" << std::endl;
    vsProj << "    <Optimize>true</Optimize>" << std::endl;
    vsProj << "    <OutputPath>bin\\Production\\</OutputPath>" << std::endl;
    vsProj << "    <DefineConstants>TRACE</DefineConstants>" << std::endl;
    vsProj << "    <ErrorReport>prompt</ErrorReport>" << std::endl;
    vsProj << "    <WarningLevel>4</WarningLevel>" << std::endl;
    vsProj << "  </PropertyGroup>" << std::endl;
    vsProj << "  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Release OpenGL|x64' \">" << std::endl;
    vsProj << "    <DebugType>pdbonly</DebugType>" << std::endl;
    vsProj << "    <Optimize>true</Optimize>" << std::endl;
    vsProj << "    <OutputPath>bin\\Release\\</OutputPath>" << std::endl;
    vsProj << "    <DefineConstants>TRACE</DefineConstants>" << std::endl;
    vsProj << "    <ErrorReport>prompt</ErrorReport>" << std::endl;
    vsProj << "    <WarningLevel>4</WarningLevel>" << std::endl;
    vsProj << "  </PropertyGroup>" << std::endl;
    vsProj << "  <ItemGroup>" << std::endl;
    vsProj << "    <Reference Include=\"System\" />" << std::endl;
    vsProj << "    <Reference Include=\"System.Numerics\" />" << std::endl;
    vsProj << "    <Reference Include=\"Library\\ScriptCore.dll\" />" << std::endl;
    vsProj << "  </ItemGroup>" << std::endl;
    vsProj << "  <ItemGroup>" << std::endl;
    vsProj << "    <Folder Include=\"Scripts\" />" << std::endl;
    vsProj << "    <Compile Include=\"Scripts\\**\\*.cs\" />" << std::endl;
    vsProj << "  </ItemGroup>" << std::endl;
    vsProj << "  <Import Project=\"$(MSBuildToolsPath)\\Microsoft.CSharp.targets\" />" << std::endl;
    vsProj << "</Project>" << std::endl;
    vsProj.close();

    // generate VS solution
    static std::string projectTypeID = "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC"; // DO NOT CHANGE, fixed type id that indicates the project is C#

    std::ofstream vsSln(GetProjectFolder().string() + "/" + GetProjectFolder().filename().string() + ".sln");
    if (!vsSln)
        return;
    vsSln << "Microsoft Visual Studio Solution File, Format Version 12.00" << std::endl;
    vsSln << "# Visual Studio Version 16" << std::endl;
    vsSln << "VisualStudioVersion = 16.0.30907.101" << std::endl;
    vsSln << "MinimumVisualStudioVersion = 10.0.40219.1" << std::endl;
    vsSln << "Project(\"{" << projectTypeID << "}\") = \"Scripting\", \"Scripting.csproj\", \"{}\"" << std::endl;
    vsSln << "EndProject" << std::endl;
    vsSln << "Global" << std::endl;
    vsSln << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << std::endl;
    vsSln << "\t\tDebug|OpenGL = Debug|OpenGL" << std::endl;
    vsSln << "\t\tProduction|OpenGL = Production|OpenGL" << std::endl;
    vsSln << "\t\tRelease|OpenGL = Release|OpenGL" << std::endl;
    vsSln << "\tEndGlobalSection" << std::endl;
    vsSln << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << std::endl;
    vsSln << "\t{}.Debug|OpenGL.ActiveCfg = Debug OpenGL|x64" << std::endl;
    vsSln << "\t{}.Debug|OpenGL.Build.0 = Debug OpenGL|x64" << std::endl;
    vsSln << "\t{}.Production|OpenGL.ActiveCfg = Production OpenGL|x64" << std::endl;
    vsSln << "\t{}.Production|OpenGL.Build.0 = Production OpenGL|x64" << std::endl;
    vsSln << "\t{}.Release|OpenGL.ActiveCfg = Release OpenGL|x64" << std::endl;
    vsSln << "\t{}.Release|OpenGL.Build.0 = Release OpenGL|x64" << std::endl;
    vsSln << "\tEndGlobalSection" << std::endl;
    vsSln << "\tGlobalSection(SolutionProperties) = preSolution" << std::endl;
    vsSln << "\tHideSolutionNode = FALSE" << std::endl;
    vsSln << "\tEndGlobalSection" << std::endl;
    vsSln << "\tGlobalSection(ExtensibilityGlobals) = postSolution" << std::endl;
    vsSln << "\tSolutionGuid = {}" << std::endl;
    vsSln << "\tEndGlobalSection" << std::endl;
    vsSln << "EndGlobal" << std::endl;
    vsSln.close();
}
