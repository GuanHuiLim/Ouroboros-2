#pragma once
#include <filesystem>
#include <string>
#include "Ouroboros/Asset/AssetManager.h"

class Project
{
public:
	static void LoadProject(std::filesystem::path& p);
	static void SaveProject();
public:
	static std::shared_ptr<oo::AssetManager> GetAssetManager() { return s_AssetManager; };
	
	static std::filesystem::path GetStartingScene() { return s_startingScene; };
	static std::filesystem::path GetProjectFolder() { return s_projectFolder; };
	static std::filesystem::path GetAssetFolder() { return s_projectFolder / s_assetFolder; };
	static std::filesystem::path GetSceneFolder() { return s_projectFolder / s_sceneFolder; };
	static std::filesystem::path GetPrefabFolder() { return s_projectFolder / s_prefabFolder; };

	static std::filesystem::path GetScriptCoreDLL() { return s_projectFolder / s_scriptcoreDLL; };
	static std::filesystem::path GetScriptModulePath() { return s_projectFolder / s_scriptmodulePath; };
	static std::filesystem::path GetScriptBuildPath() { return s_projectFolder / s_scriptbuildPath; };
private:
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
