project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    --debugformat "c7"
    warnings "Extra" -- Set warnings level to 4 for this project
    
    -- Engine output directory
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    --precompiled headers for engine
    pchheader "pch.h"
    pchsource "src/pch.cpp"
    
    --Disable PCH for vendor files
    filter "files:vendor/**/**.cpp"
        flags { "NoPCH" }
    filter "files:vendor/**/**.c"
        flags { "NoPCH" }
    filter {}   -- resets the filter

    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.c",
        "src/**.cpp",
    }

    -- defines
    -- {
    --     "_CRT_SECURE_NO_WARNINGS"
    -- }

    includedirs
    {
        "src",

        "%{IncludeDir.spdlog}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.SDL}",
        
        "%{IncludeDir.imgui}",

        "%{IncludeDir.glm}",
        "%{IncludeDir.rapidjson}",
        "%{IncludeDir.rttr}",

        "%{IncludeDir.launcher}",
        "%{IncludeDir.ecs}",

        "%{IncludeDir.sharedlib}",

        --for tracy
        "%{IncludeDir.tracy}",
    }

    -- library diretories
    -- CLIENT : Only Add client required Directories, rest settled by references
    libdirs 
    {
        "%{LibraryDir.VulkanSDK}",
        "%{LibraryDir.SDL}",
        "%{LibraryDir.rttr}/Debug",
        "%{LibraryDir.rttr}/Release",
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
        "%{Library.Vulkan}",
        "ImGui",
        "SDL2",
        "SDL2main",
        "SDL2test",
        
        "ECS",
        "Launcher",
        "SharedLib",
        
        "dbghelp",
        --"srcsrv", are these even needed? might just remove-em altogether.
        --"symsrv",

    }
    
    filter "system:windows"
        cppdialect "C++20"
        staticruntime "off"
        systemversion "latest"

        -- Game Engine icon
        files { 'Editor.rc', '**.ico' }
        vpaths { ['Resources/*'] = { '*.rc', '**.ico' } }
        
        defines
        {
            "OO_PLATFORM_WINDOWS",
            "EDITOR_PLATFORM_WINDOWS",
        }
        
        --enable this post build command for 64 bit system
        architecture "x86_64"
        postbuildcommands
        {
            -- SDL2.0 
            {"{COPY} \"%{AppVendor}/sdl2/lib/x64/SDL2.dll\" " .. binApp },
            -- Controller Support file
            {"{COPY} \"%{AppDir}/gamecontrollerdb.txt\" " .. binApp },
            -- ImGui Default Settings
            {"{COPY} \"%{AppDir}/default.ini\" " .. binApp },
            -- copy General DLLs
            {"{COPY} \"%{AppDir}/dlls/\" " .. binApp },
            
            -- copy launcher's Data file
            {"{COPY} \"%{AppVendor}/launcher/Oroborous-Launcher/Launcher/BaseTemplate\" " .. binApp },
        }
    

    filter{ "configurations:Debug", "platforms:Editor"}
        defines { "EDITOR_DEBUG", "TRACY_ENABLE", "TRACY_ON_DEMAND" }
    filter{ "configurations:Release", "platforms:Editor"}
        defines { "EDITOR_RELEASE", "TRACY_ENABLE", "TRACY_ON_DEMAND" }
    filter{ "configurations:Production", "platforms:Editor"}
        defines "EDITOR_PRODUCTION"
    filter{}
    
    filter "configurations:Debug"
        defines "OO_DEBUG"
        symbols "On"

        architecture "x86_64"
        -- Copy neccesary DLLs to output directory
        postbuildcommands
        {
            {"{COPY} \"%{LibraryDir.rttr}/Debug/rttr_core_d.dll\" " .. binApp},
        }

        links
        {
            "rttr_core_d",
        }
        
    filter "configurations:Release"
        defines "OO_RELEASE"
        optimize "On"

        architecture "x86_64"
        -- Copy neccesary DLLs to output directory
        postbuildcommands
        {
            {"{COPY} \"%{LibraryDir.rttr}/Release/rttr_core.dll\" " .. binApp},
        }

        links
        {
            "rttr_core",
        }
        
    filter "configurations:Production"
        defines "OO_PRODUCTION"
        optimize "On"
        
        architecture "x86_64"
        -- Copy neccesary DLLs to output directory
        postbuildcommands
        {
            {"{COPY} \"%{LibraryDir.rttr}/Release/rttr_core.dll\" " .. binApp},
        }

        links
        {
            "rttr_core",
        }
