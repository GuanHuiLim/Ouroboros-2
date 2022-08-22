#pragma once
#include <filesystem>
#include <string>
class Project
{
public:
	static void LoadProject(std::filesystem::path& p);
	static void SaveProject();
public:
	static std::filesystem::path GetStartingScene() { return s_startingScene; };
	static std::filesystem::path GetProjectFolder() { return s_projectFolder; };
	static std::filesystem::path GetSceneFolder() { return s_projectFolder.string() + s_sceneFolder.string(); };
	static std::filesystem::path GetScriptCoreDLL() { return s_projectFolder.string() +s_scriptcoreDLL.string(); };
	static std::filesystem::path GetScriptModulePath() { return s_projectFolder.string() + s_scriptmodulePath.string(); };
	static std::filesystem::path GetScriptBuildPath() { return s_projectFolder.string() + s_scriptbuildPath.string(); };
private:
	
	inline static std::filesystem::path s_configFile;
	inline static std::filesystem::path s_startingScene;
	inline static std::filesystem::path s_projectFolder;
	inline static std::filesystem::path s_sceneFolder;
	inline static std::filesystem::path s_scriptcoreDLL;
	inline static std::filesystem::path s_scriptmodulePath;
	inline static std::filesystem::path s_scriptbuildPath;
};
