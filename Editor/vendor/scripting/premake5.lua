-- Core Scripting Project
project "Scripting"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    debugformat "c7"
    warnings "Extra" -- Set warnings level to 4 for all projects.

    -- Engine output directory
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    -- Engine's files
    files
    {
        "src/**.h",
        "src/**.cpp",
    }

    -- Engine's defines 
    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
    }

    -- Engine's include directories
    includedirs
    {
        "src",
        "vendor/mono/include/mono-2.0",
    }

    -- library diretories
    libdirs 
    {
        "vendor/mono/lib",
    }

    -- linking External libraries 
    -- NOTE: do not put their extensions.
    -- IMPORTANT DISCOVERY
    --  For visual studio, If the name of the project is selected it will be linekd via 
    --  VS references directly against other projects.
    --  doesn't show up as links in project properties but instead
    --  can be found on Build Dependencies/Add Reference options
    links
    {
        "mono-2.0-sgen",
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