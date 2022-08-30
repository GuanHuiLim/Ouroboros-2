-- This is a static library of your vulkan files that will be interacting with
-- the Engine, so only expose the files you need, not all files need to be included
-- and try to make them compile
-- this static lib will be linked to the editor in the end so that the editor can
-- run the relevant API functions
project "Vulkan"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    -- output directory
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	

    files
    {
		
        -- include the relevant files you want exposed to the Editor here
        "OO_Vulkan/src/**.*",
    }
    excludes{"OO_Vulkan/src/main.cpp"}
	
    includedirs
    {
        -- include directories of the external dependencies that you need for the above files to compile
        
        "%{IncludeDir.glm}",
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.vulkanSrc}",
        "%{IncludeDir.assimp}",
        -- "%{IncludeDir.vulkanIMGUI}",
        "%{IncludeDir.assimpBin}",

        -- for the case of imgui if you want it to be direct just 
        -- append to the back so that it compiles your stuff properly
        "%{IncludeDir.imgui}",

        "%{IncludeDir.VulkanSDK}",

        -- assimp will need to be here too [will probably need to put assimp properly in the editor files itself first]
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