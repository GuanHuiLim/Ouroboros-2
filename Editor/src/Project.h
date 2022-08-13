#pragma once
#include <filesystem>
#include <string>
class Project
{
public:
	static void LoadProject(std::filesystem::path& p);
public:
	static std::string GetStartingScene() { return s_startingScene; };
	static std::string GetProjectFolder() { return s_projectFolder; };
	static std::string GetSceneFolder() { return s_sceneFolder; };
	static std::string GetScriptCoreDLL() { return s_scriptcoreDLL; };
	static std::string GetScriptModulePath() { return s_scriptmodulePath; };
	static std::string GetScriptBuildPath() { return s_scriptbuildPath; };
private:

	inline static std::string s_startingScene;
	inline static std::string s_projectFolder;
	inline static std::string s_sceneFolder;
	inline static std::string s_scriptcoreDLL;
	inline static std::string s_scriptmodulePath;
	inline static std::string s_scriptbuildPath;
};
