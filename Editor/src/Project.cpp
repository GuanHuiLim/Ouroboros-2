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
#include "App/Editor/Properties/SerializerProperties.h"
#include "Ouroboros/Vulkan/GlobalRendererSettings.h"
#include "App/Editor/Properties/UI_RTTRType.h"

#include "Ouroboros/Physics/RigidbodyComponent.h"

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

	//scenes to add to scene manager
	oo::RuntimeController::container_type m_loadpaths;
	auto scenes_settings = doc.FindMember("Scenes");
	for (auto iter = scenes_settings->value.MemberBegin(); iter != scenes_settings->value.MemberEnd(); ++iter)
	{
		m_loadpaths.emplace_back(oo::SceneInfo{ iter->name.GetString() , s_projectFolder.string() + s_sceneFolder.string() + iter->value.GetString() });
	}
	LoadProjectEvent lpe(std::move(s_projectFolder.string() + s_sceneFolder.string() + s_startingScene.string()), std::move(m_loadpaths),std::move(s_projectFolder.string()));
	oo::EventManager::Broadcast(&lpe);

	//load renderer settings
	if (doc.HasMember("Renderer Settings"))
	{
		LoadRenderer(doc.FindMember("Renderer Settings")->value);
	}


	//end
	ifs.close();

	//load assets here
	std::filesystem::path hard_assetfolderpath = GetAssetFolder();
	s_AssetManager = std::make_shared<oo::AssetManager>(hard_assetfolderpath);
	s_AssetManager->GetOrLoadDirectoryAsync(hard_assetfolderpath, true);

	//load input manager
	LoadInputs(GetProjectFolder() / InputFileName);
	//load script sequence
	LoadScriptSequence(GetScriptSequencePath());
	//load layernames
	LoadLayerNames();
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
	//auto size = scenes->value.MemberCount();
	auto* runtimecontroller = ImGuiManager::s_runtime_controller;
	auto loadpaths = runtimecontroller->GetLoadPaths();
	for(auto scene_info : loadpaths)
	{
		rapidjson::Value name(scene_info.SceneName.c_str(),doc.GetAllocator());
		std::filesystem::path scene_loadpath = std::filesystem::relative(scene_info.LoadPath, GetSceneFolder());
		rapidjson::Value data(scene_loadpath.string().c_str(),doc.GetAllocator());
		scenes->value.AddMember(name, data, doc.GetAllocator());
	}
	
	if (doc.HasMember("Renderer Settings"))
	{
		auto& val = doc.FindMember("Renderer Settings")->value;
		val.RemoveAllMembers();//its easier to clear everything and key in the value again
		SaveRenderer(val, doc);
	}
	else
	{
		rapidjson::Value val(rapidjson::kObjectType);
		SaveRenderer(val,doc);
		doc.AddMember("Renderer Settings", val, doc.GetAllocator());
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
	SaveInputs(GetProjectFolder() / (InputFileName));
	SaveScriptSequence(GetScriptSequencePath());
	SaveLayerNames();
}

void Project::LoadInputs(const std::filesystem::path& loadpath)
{
	std::ifstream ifs2(loadpath);
	if (!ifs2)
	{
		// load input manager if there is nothing
		oo::InputManager::LoadDefault();
		return;
	}
	rapidjson::IStreamWrapper isw2(ifs2);
	rapidjson::Document input_doc;
	input_doc.ParseStream(isw2);
	SerializerLoadProperties loadproperties;

    std::vector<oo::InputAxis> InputManager_Axis;
	//auto& InputManager_Axis = oo::InputManager::GetAxes();
	rttr::type t = rttr::type::get<oo::InputAxis>();
	for (auto iter = input_doc.MemberBegin(); iter != input_doc.MemberEnd(); ++iter)
	{
		oo::InputAxis axis;
		for (auto members = iter->value.MemberBegin(); members != iter->value.MemberEnd(); ++members)
		{
			rttr::property prop = t.get_property(members->name.GetString());
			auto types_ui_rttr = UI_RTTRType::types.find(prop.get_type().get_id());
			if (types_ui_rttr == UI_RTTRType::types.end())
			{
				if (prop.get_type() == rttr::type::get<oo::InputAxis::Settings>())
				{
					auto arr = members->value.GetArray();
					oo::InputAxis::Settings setting;
					setting.negativeButton = arr[0].GetUint();
					setting.positiveButton = arr[1].GetUint();
					setting.negativeAltButton = arr[2].GetUint();
					setting.positiveAltButton = arr[3].GetUint();
					setting.pressesRequired = arr[4].GetUint();
					setting.maxGapTime = arr[5].GetFloat();
					setting.holdDurationRequired = arr[6].GetFloat();
                    setting.invert = arr[7].GetBool();
                    setting.onPressOnly = arr[8].GetBool();
					prop.set_value(axis, setting);
					continue;
				}
				else
				{
					ASSERT_MSG(true, "type not supported");
					continue;
				}
			}
			auto loadprop_iter = loadproperties.m_load_commands.find(types_ui_rttr->second);
			if (loadprop_iter == loadproperties.m_load_commands.end())
			{
				ASSERT_MSG(true, "type not supported");
				continue;
			}
			rttr::variant var = prop.get_value(axis);
			loadprop_iter->second(var, std::move(members->value));
			prop.set_value(axis, var);
		}
		InputManager_Axis.push_back(axis);
	}
	ifs2.close();
    oo::InputManager::Load(InputManager_Axis);
	
}


void Project::SaveInputs(const std::filesystem::path& savepath)
{
	rapidjson::Document input_doc;
	auto& doc_object = input_doc.SetObject();
	SerializerSaveProperties saveproperties;

	//auto obj = input_doc.GetObj();
	auto& InputManager_Axis = oo::InputManager::GetAxes();
	rttr::type t = rttr::type::get<oo::InputAxis>();
	auto properties = t.get_properties();
	for (auto& axes : InputManager_Axis)
	{
		rapidjson::Value values(rapidjson::kObjectType);
		for (rttr::property prop : properties)
		{
			auto types_ui_rttr = UI_RTTRType::types.find(prop.get_type().get_id());
			if (types_ui_rttr == UI_RTTRType::types.end())
			{
				if (prop.get_type() == rttr::type::get<oo::InputAxis::Settings>())
				{
					rapidjson::Value setting(rapidjson::kArrayType);
					auto axes_Setting = prop.get_value(axes).get_value<oo::InputAxis::Settings>();
					setting.PushBack(axes_Setting.negativeButton, input_doc.GetAllocator());
					setting.PushBack(axes_Setting.positiveButton, input_doc.GetAllocator());
					setting.PushBack(axes_Setting.negativeAltButton, input_doc.GetAllocator());
					setting.PushBack(axes_Setting.positiveAltButton, input_doc.GetAllocator());
					setting.PushBack(axes_Setting.pressesRequired, input_doc.GetAllocator());
					setting.PushBack(axes_Setting.maxGapTime, input_doc.GetAllocator());
					setting.PushBack(axes_Setting.holdDurationRequired, input_doc.GetAllocator());
                    setting.PushBack(axes_Setting.invert, input_doc.GetAllocator());
                    setting.PushBack(axes_Setting.onPressOnly, input_doc.GetAllocator());

					values.AddMember(rapidjson::Value(prop.get_name().data(), input_doc.GetAllocator()), setting, input_doc.GetAllocator());
					continue;
				}
				else
				{
					ASSERT_MSG(true, "type not supported");
					continue;
				}
			}
			auto saveprop_iter = saveproperties.m_save_commands.find(types_ui_rttr->second);
			if (saveprop_iter == saveproperties.m_save_commands.end())
			{
				ASSERT_MSG(true, "type not supported");
				continue;
			}

			saveprop_iter->second(input_doc, values, prop.get_value(axes), prop);
			//values.AddMember(rapidjson::Value(prop.get_name().data(), input_doc.GetAllocator()), setting, input_doc.GetAllocator());
		}
		doc_object.AddMember(rapidjson::Value(axes.GetName().c_str(), input_doc.GetAllocator()), values, input_doc.GetAllocator());
	}

	std::ofstream ofs2(savepath);
	if (!ofs2)
		return;
	rapidjson::OStreamWrapper osw(ofs2);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
	input_doc.Accept(writer);
	ofs2.close();
}

void Project::SaveScriptSequence(const std::filesystem::path& path)
{
	std::ofstream ofs(path);
	if (!ofs)
		return;
	
	rapidjson::Document input_doc;
	auto& doc_object = input_doc.SetObject();

	rapidjson::Value pre_arr(rapidjson::kArrayType);
	for (auto& before : oo::ScriptManager::GetBeforeDefaultOrder())
	{
		pre_arr.PushBack(rapidjson::Value(before.ToString().c_str(),input_doc.GetAllocator())
						, input_doc.GetAllocator());
	}
	doc_object.AddMember("Before Default Order", pre_arr, input_doc.GetAllocator());

	rapidjson::Value post_arr(rapidjson::kArrayType);
	for (auto& after : oo::ScriptManager::GetAfterDefaultOrder())
	{
		post_arr.PushBack(rapidjson::Value(after.ToString().c_str(), input_doc.GetAllocator())
			, input_doc.GetAllocator());
	}
	doc_object.AddMember("After Default Order", post_arr, input_doc.GetAllocator());


	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
	input_doc.Accept(writer);
	ofs.close();
}

void Project::LoadScriptSequence(const std::filesystem::path& path)
{
	oo::ScriptManager::ClearScriptExecutionOrder();
	std::ifstream ifs2(path);
	if (!ifs2)
	{
		return;
	}
	rapidjson::IStreamWrapper isw2(ifs2);
	rapidjson::Document input_doc;
	input_doc.ParseStream(isw2);
	
	rapidjson::Value& before_order = input_doc.GetObj().FindMember("Before Default Order")->value;
	rapidjson::GenericArray before_arr = before_order.GetArray();
	for (rapidjson::SizeType i = 0 ; i < before_arr.Size();++i)
	{
		oo::ScriptManager::InsertBeforeDefaultOrder(oo::ScriptClassInfo(before_arr[i].GetString()));
	}

	rapidjson::Value& after_order = input_doc.FindMember("After Default Order")->value;
	rapidjson::GenericArray after_arr = after_order.GetArray();
	for (rapidjson::SizeType i = 0; i < after_arr.Size(); ++i)
	{
		oo::ScriptManager::InsertAfterDefaultOrder(oo::ScriptClassInfo(after_arr[i].GetString()));
	}
	ifs2.close();
}

void Project::LoadRendererSettingFile()
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
	if (doc.HasMember("Renderer Settings"))
	{
		LoadRenderer(doc.FindMember("Renderer Settings")->value);
	}
}

void Project::SaveRendererSettingFile()
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
	if (doc.HasMember("Renderer Settings"))
	{
		auto& val = doc.FindMember("Renderer Settings")->value;
		val.RemoveAllMembers();//its easier to clear everything and key in the value again
		SaveRenderer(val, doc);
	}
	ifs.close();
}

void Project::LoadLayerNames()
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
	if (doc.HasMember("Layer Names"))
	{
		auto& val = doc.FindMember("Layer Names")->value;
		auto arr = val.GetArray();
		unsigned largersize = std::min((unsigned)oo::RigidbodyComponent::LayerNames.size(), (unsigned)arr.Size());
		for (unsigned i = 0; i < largersize; ++i)
		{
			oo::RigidbodyComponent::LayerNames[i] = arr[i].GetString();
		}
		int i = 0;
		i = i + 1;
	}
	ifs.close();
}

void Project::SaveLayerNames()
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
	rapidjson::Value& doc_val = doc.GetObj();
	if (doc_val.HasMember("Layer Names"))
	{
		auto& val = doc_val.FindMember("Layer Names")->value;
		auto arr = val.GetArray();
		arr.Clear();
		rapidjson::Value names(rapidjson::kArrayType);
		for (auto layer : oo::RigidbodyComponent::LayerNames)
		{
			arr.PushBack(rapidjson::Value(layer.c_str(), layer.size(),doc.GetAllocator()), doc.GetAllocator());
		}
	}
	else
	{
		rapidjson::Value names(rapidjson::kArrayType);
		for (auto& layer : oo::RigidbodyComponent::LayerNames)
		{
			names.PushBack(rapidjson::Value(layer.c_str(),layer.size(),doc.GetAllocator()), doc.GetAllocator());
		}
		doc_val.AddMember("Layer Names", names,doc.GetAllocator());
	}
	ifs.close();


	std::ofstream ofs(s_configFile);
	if (!ofs)
		return;
	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
	doc.Accept(writer);
	ofs.close();
}

void Project::LoadRendererSetting(rapidjson::Value& setting_val, rttr::variant& v)
{
	static SerializerLoadProperties loadProperties;
	rttr::type t = v.get_type();
	for (auto iter = setting_val.MemberBegin(); iter != setting_val.MemberEnd(); ++iter)
	{
		rttr::property prop = t.get_property(iter->name.GetString());
		auto typeiter = UI_RTTRType::types.find(prop.get_type().get_id());
		if (typeiter == UI_RTTRType::types.end())
			continue;
		auto commanditer = loadProperties.m_load_commands.find(typeiter->second);
		if (commanditer == loadProperties.m_load_commands.end())
			continue;

		rttr::variant variant;
		commanditer->second(variant, std::move(iter->value));
		prop.set_value(v, variant);
	}
}

void Project::LoadRenderer(rapidjson::Value& val)
{
	rttr::type t = oo::RendererSettings::setting.get_type();
	for (auto iter = val.MemberBegin(); iter != val.MemberEnd(); ++iter)
	{
		std::string name = iter->name.GetString();
		rttr::property prop = t.get_property(name);
		/*if (prop.is_valid() == false)
			continue;*/
		rttr::variant v = prop.get_value(oo::RendererSettings::setting);
		LoadRendererSetting(iter->value, v);
		prop.set_value(oo::RendererSettings::setting, v);
	}
}

void Project::SaveRendererSetting(rapidjson::Value& val,rttr::property _prop, rttr::variant v, rapidjson::Document& doc)
{
	static SerializerSaveProperties saveProperties;
	rttr::type t = v.get_type();
	rapidjson::Value setting_val(rapidjson::kObjectType);
	for (auto prop : t.get_properties())
	{
		auto typeiter = UI_RTTRType::types.find(prop.get_type().get_id());
		if (typeiter == UI_RTTRType::types.end())
			continue;
		auto commanditer = saveProperties.m_save_commands.find(typeiter->second);
		if (commanditer == saveProperties.m_save_commands.end())
			continue;
		commanditer->second(doc, setting_val, prop.get_value(v), prop);
	}
	val.AddMember(rapidjson::Value(_prop.get_name().data(), doc.GetAllocator()), setting_val, doc.GetAllocator());
}

void Project::SaveRenderer(rapidjson::Value& val, rapidjson::Document& doc)
{
	for (auto prop : oo::RendererSettings::setting.get_type().get_properties())
	{
		rttr::variant v = prop.get_value(oo::RendererSettings::setting);
		SaveRendererSetting(val,prop,v, doc);
	}
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

    // csproj doesn't exist, create csproj
    if (!std::filesystem::exists(GetScriptModulePath()))
    {
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
        vsProj << "    <DebugType>portable</DebugType>" << std::endl;
        vsProj << "    <Optimize>false</Optimize>" << std::endl;
        vsProj << "    <OutputPath>bin\\Debug\\</OutputPath>" << std::endl;
        vsProj << "    <DefineConstants></DefineConstants>" << std::endl;
        vsProj << "    <ErrorReport>prompt</ErrorReport>" << std::endl;
        vsProj << "    <WarningLevel>4</WarningLevel>" << std::endl;
        vsProj << "  </PropertyGroup>" << std::endl;
        vsProj << "  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Production OpenGL|x64' \">" << std::endl;
        vsProj << "    <DebugType>portable</DebugType>" << std::endl;
        vsProj << "    <Optimize>true</Optimize>" << std::endl;
        vsProj << "    <OutputPath>bin\\Production\\</OutputPath>" << std::endl;
        vsProj << "    <DefineConstants>TRACE</DefineConstants>" << std::endl;
        vsProj << "    <ErrorReport>prompt</ErrorReport>" << std::endl;
        vsProj << "    <WarningLevel>4</WarningLevel>" << std::endl;
        vsProj << "  </PropertyGroup>" << std::endl;
        vsProj << "  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Release OpenGL|x64' \">" << std::endl;
        vsProj << "    <DebugType>portable</DebugType>" << std::endl;
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
    }
    
    static std::string projectTypeID = "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC"; // DO NOT CHANGE, fixed type id that indicates the project is C#
    std::string slnPath = GetProjectFolder().string() + "/" + GetProjectFolder().filename().string() + ".sln";
    // sln doesn't exist, create sln
    if (!std::filesystem::exists(slnPath))
    {
        // generate VS solution
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
}
