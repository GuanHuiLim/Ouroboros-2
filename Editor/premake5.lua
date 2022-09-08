project "Editor"
    kind "ConsoleApp"
    language "C++"
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
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.assimpBin}",
        "%{IncludeDir.SDL}",
        
        "%{IncludeDir.imgui}",

        "%{IncludeDir.glm}",
        "%{IncludeDir.rapidjson}",
        "%{IncludeDir.rttr}",

        "%{IncludeDir.launcher}",
        "%{IncludeDir.ecs}",

        "%{IncludeDir.sharedlib}",
		
		"%{IncludeDir.mono}",
		"%{IncludeDir.scripting}",

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
        "%{LibraryDir.assimp}/Release",
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
		
		--"mono-2.0-sgen",
		"Scripting",
		"ScriptCore",
        
        --Linking to vulkan Library [Uncomment the next line when youre done setting up]
        "Vulkan",
		"assimp-vc142-mt",

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
        
        -- if Editor need any prebuild commands regardless of debug/release/production
        prebuildcommands
        {
			{"call \"%{AppVendor}/vulkan/OO_Vulkan/shaders/compileShaders.bat\"" }
        }

        -- if Editor needs any postbuild commands regardless of debug/release/production
        postbuildcommands
        {
			-- [IMPORTANT] copy command requires a space after the target directory.
            -- SDL2.0 
            {"{COPY} \"%{AppVendor}/sdl2/lib/x64/SDL2.dll\" \"" .. binApp .. "\""},
            -- Controller Support file
            {"{COPY} \"%{AppDir}/gamecontrollerdb.txt\" \"" .. binApp .. "\""},
            -- ImGui Default ini
            {"{COPY} \"%{AppDir}/default.ini\" \"" .. binApp .. "\""},
			-- ImGui Default Style Settings
            {"{COPY} \"%{AppDir}/EditorMode.settings\" \"" .. binApp .. "\""},
			-- ImGui Default Style Settings
            {"{COPY} \"%{AppDir}/PlayMode.settings\" \"" .. binApp .. "\""},
            -- copy General DLLs
            {"{COPY} \"%{AppDir}/dlls/\" \"" .. binApp .. "\"" },
			-- copy Editor Icons Folder in its entirety.
			{ "mkdir \"" .. binApp .. "/Icons\"" },
			{"{COPY} \"%{AppDir}/icons\" \"" .. binApp .. "/Icons\"" },
            -- copy launcher's Data file
            {"{COPY} \"%{AppVendor}/launcher/Oroborous-Launcher/Launcher/BaseTemplate\" \"" .. binApp .. "\"" },
            -- tracy server copy 
            {"{COPY} \"%{AppDir}/tracy_server\" \"" .. binApp .. "/tracy_server\""}, 
			-- vulkan shaders copy
			{ "mkdir \"" .. binApp .. "/shaders/bin\"" },
            {"{COPY} \"%{AppVendor}/vulkan/OO_Vulkan/shaders/bin\" \"" .. binApp .. "/shaders/bin\""}, 			
			{ "mkdir \"" .. AppDir .. "/shaders/bin\"" },
            {"{COPY} \"%{AppVendor}/vulkan/OO_Vulkan/shaders/bin\" \"" .. AppDir .. "/shaders/bin\""}, 
        }
    
        -- if editor needs to link with any static/dynamic library regardless of debug/release/production
        links
        {

        }

    filter{ "configurations:Debug", "platforms:Editor"}
        defines { "EDITOR_DEBUG", "TRACY_ENABLE", "TRACY_ON_DEMAND" }
    filter{ "configurations:Release", "platforms:Editor"}
        defines { "EDITOR_RELEASE", "TRACY_ENABLE", "TRACY_ON_DEMAND" }
    filter{ "configurations:Production", "platforms:Editor"}
        defines { "EDITOR_PRODUCTION", "TRACY_ENABLE", "TRACY_ON_DEMAND" }
    filter{}
    
    filter "configurations:Debug"
        runtime "Debug" -- uses the debug Runtime Library
        defines "OO_DEBUG"
        symbols "On"
        architecture "x86_64"
        
        -- Copy neccesary DLLs to output directory
        postbuildcommands
        {
            -- [IMPORTANT] copy command requires a space after the target directory.
            {"{COPY} \"%{LibraryDir.rttr}/Debug/rttr_core_d.dll\" \"" .. binApp .. "\""},
        }

        links
        {
            "rttr_core_d",
        }
    
    filter "configurations:Release"
        runtime "Release" -- uses the release Runtime Library
        defines "OO_RELEASE"
        optimize "On"
        architecture "x86_64"

        -- Copy neccesary DLLs to output directory
        postbuildcommands
        {
				-- [IMPORTANT] copy command requires a space after the target directory.
            {"{COPY} \"%{LibraryDir.rttr}/Release/rttr_core.dll\" \"" .. binApp .. "\""},
        }

        links
        {
            "rttr_core",
        }
        
    filter "configurations:Production"
        runtime "Release" -- uses the release Runtime Library
        defines "OO_PRODUCTION"
        optimize "On"
        architecture "x86_64"

        -- Copy neccesary DLLs to output directory
        postbuildcommands
        {
				-- [IMPORTANT] copy command requires a space after the target directory.
            {"{COPY} \"%{LibraryDir.rttr}/Release/rttr_core.dll\" \"" .. binApp .. "\""},
        }

        links
        {
            "rttr_core",
        }
