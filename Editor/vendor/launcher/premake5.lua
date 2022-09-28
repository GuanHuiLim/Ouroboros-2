project "Launcher"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    -- output directory
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Oroborous-Launcher/Launcher/Launcher/*.cpp",
        "Oroborous-Launcher/Launcher/Launcher/*.h",
        "Oroborous-Launcher/Launcher/Utilities/*.h",
        "Oroborous-Launcher/Launcher/Utilities/*.cpp",
    }

    includedirs
    {
        "Oroborous-Launcher/Launcher",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.rapidjson}",
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
        defines "NDEBUG"