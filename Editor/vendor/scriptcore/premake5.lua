project "ScriptCore"
	kind "SharedLib"
	language "C#"

	-- copy over to Editor's Engine/Scriptcore file
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{AppDir}/dlls")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cs", 
	}
	
	links
	{
		"System",
		"System.Numerics",
	}