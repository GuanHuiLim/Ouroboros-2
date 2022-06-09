project "Launcher"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    -- output directory
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Oroborous-Launcher/Launcher/examples/example_glfw_opengl3/Launcher/*.cpp",
        "Oroborous-Launcher/Launcher/examples/example_glfw_opengl3/Launcher/*.h",
        "Oroborous-Launcher/Launcher/examples/example_glfw_opengl3/Utilities/*.h",
        "Oroborous-Launcher/Launcher/examples/example_glfw_opengl3/Utilities/*.cpp",
    }

    includedirs
    {
        "Oroborous-Launcher/Launcher",
        "Oroborous-Launcher/Launcher/examples/example_glfw_opengl3",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.rapidjson}",
    }

    filter "system:windows"
        systemversion "latest"
        cppdialect "C++17"

    filter "system:linux"
        pic "On"
        systemversion "latest"
        cppdialect "C++17"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"