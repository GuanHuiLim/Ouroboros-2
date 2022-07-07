project "SharedLib"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    -- output directory
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Isolated-Testing-Ground/Quaternion/include/*.*",
        "Isolated-Testing-Ground/Quaternion/src/*.*",

        "Isolated-Testing-Ground/SceneManagement/include/*.*",
        "Isolated-Testing-Ground/SceneManagement/src/*.*",

        
        "Isolated-Testing-Ground/Scenegraph/include/*.*",
        "Isolated-Testing-Ground/Scenegraph/src/*.*",
    }

    includedirs
    {
        "%{IncludeDir.scenegraph}",
        "%{IncludeDir.scene}",
        "%{IncludeDir.quaternion}",
        --"Oroborous-Launcher/Launcher",
        --"%{IncludeDir.imgui}",
        --"%{IncludeDir.rapidjson}",
        "%{IncludeDir.glm}",
    }

    filter "system:windows"
        systemversion "latest"
        cppdialect "C++20"

    filter "system:linux"
        pic "On"
        systemversion "latest"
        cppdialect "C++20"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"