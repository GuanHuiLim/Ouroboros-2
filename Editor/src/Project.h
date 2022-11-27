#pragma once
#include <filesystem>
#include <string>
#include "Ouroboros/Asset/AssetManager.h"
#include "rapidjson/document.h"
class Project
{
	static constexpr const char* InputFileName = "InputBindings";
	static constexpr const char* ScriptSequenceFileName = "ScriptSequence";
public:
	static void LoadProject(std::filesystem::path& p);
	static void SaveProject();
public:
	static std::shared_ptr<oo::AssetManager> GetAssetManager() { return s_AssetManager; };
	
	//input
	static void LoadInputs(const std::filesystem::path& loadpath);
	static void SaveInputs(const std::filesystem::path& savepath);
	//script sequence
	static void SaveScriptSequence(const std::filesystem::path& path);
	static void LoadScriptSequence(const std::filesystem::path& path);
	//renderer settings
	static void LoadRendererSettingFile();
	static void SaveRendererSettingFile();

	static std::filesystem::path GetInputFilePath() { return GetProjectFolder() / InputFileName; };
	static std::filesystem::path GetScriptSequencePath() { return GetProjectFolder() / ScriptSequenceFileName; };
	static std::filesystem::path GetStartingScene() { return s_startingScene; };
	static std::filesystem::path GetProjectFolder() { return s_projectFolder; };
	static std::filesystem::path GetAssetFolder() { return s_projectFolder.string() + s_assetFolder.string(); };
	static std::filesystem::path GetSceneFolder() { return s_projectFolder.string() + s_sceneFolder.string(); };
	static std::filesystem::path GetPrefabFolder() { return s_projectFolder.string() + s_prefabFolder.string(); };

	static std::filesystem::path GetScriptCoreDLL() { return s_projectFolder.string() + s_scriptcoreDLL.string(); };
	static std::filesystem::path GetScriptModulePath() { return s_projectFolder.string() + s_scriptmodulePath.string(); };
	static std::filesystem::path GetScriptBuildPath() { return s_projectFolder.string() + s_scriptbuildPath.string(); };
private:
	
	static void LoadRendererSetting(rapidjson::Value& val, rttr::variant& v);
	static void LoadRenderer(rapidjson::Value& val);

	static void SaveRendererSetting(rapidjson::Value& val, rttr::property prop ,rttr::variant v, rapidjson::Document& doc);
	static void SaveRenderer(rapidjson::Value& val, rapidjson::Document& doc);
    
	static void UpdateScriptingFiles();
	
	inline static std::shared_ptr<oo::AssetManager> s_AssetManager = nullptr;
	inline static std::filesystem::path s_configFile;
	inline static std::filesystem::path s_startingScene;

	inline static std::filesystem::path s_assetFolder;
	inline static std::filesystem::path s_prefabFolder;
	inline static std::filesystem::path s_projectFolder;
	inline static std::filesystem::path s_sceneFolder;
	inline static std::filesystem::path s_scriptcoreDLL;
	inline static std::filesystem::path s_scriptmodulePath;
	inline static std::filesystem::path s_scriptbuildPath;
};
