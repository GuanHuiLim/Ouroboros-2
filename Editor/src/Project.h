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
	static std::filesystem::path GetSceneFolder() { return s_sceneFolder; };
	static std::filesystem::path GetScriptCoreDLL() { return s_scriptcoreDLL; };
	static std::filesystem::path GetScriptModulePath() { return s_scriptmodulePath; };
	static std::filesystem::path GetScriptBuildPath() { return s_scriptbuildPath; };
private:		
	inline static std::filesystem::path s_configFile;
	inline static std::filesystem::path s_startingScene;
	inline static std::filesystem::path s_projectFolder;
	inline static std::filesystem::path s_sceneFolder;
	inline static std::filesystem::path s_scriptcoreDLL;
	inline static std::filesystem::path s_scriptmodulePath;
	inline static std::filesystem::path s_scriptbuildPath;
};
