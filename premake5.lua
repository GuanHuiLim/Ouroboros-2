-- premake 5 uses Lua scripting
-- single line comment
-- indentations are arbitary [tabs and spaces are not considered]
-- Always use forward slash '/' : premake auto converts to the appropriate slash 

--include "./vendor/premake/premake_customization/solution_items.lua"
include "dependencies.lua"

workspace "Ouroboros"
    
    architecture "x86_64"
    startproject "Editor"   -- set startup project
    toolset "v142"          -- toolset v142 = visual studio 2019

    configurations
    {
        "Debug",
        "Release",
        "Production",
    }

    platforms
    {
        "Editor",
        "Executable",
    }

    -- solution level defines regardless of platform or configuration
    -- Note : this should be defined throughout as the consistency is required
    -- for proper compilation and functionality.
    defines
    {
        "GLM_FORCE_SILENT_WARNINGS",
        "GLM_FORCE_PURE",
        "GLM_FORCE_SSE42",
        "GLM_FORCE_DEFAULT_ALIGNED_GENTYPES", 

        "RAPIDJSON_SSE42",
    }

    --platform defines
    filter{ "platforms:Executable"}
        defines "OO_EXECUTABLE"
    filter{}
    
    filter{ "platforms:Editor"}
        defines "OO_EDITOR"
    filter{}

    filter{ "configurations:Production", "platforms:Executable"}
        defines { "OO_END_PRODUCT" }
    -- ONLY UNCOMMENT FOR TESTING
    -- filter{ "configurations:Debug", "platforms:Executable"}
    --     defines { "OO_END_PRODUCT" }
    filter{}

    flags
    {
        "MultiProcessorCompile", -- enable multicore compilation
    }
    
    -- Solution Level Disable Warning 
    disablewarnings
    {
        "4201" -- nameless struct
    }

group "Dependencies"
include "Editor/vendor/imgui"
include "Editor/vendor/launcher"
--include "Editor/vendor/Archetypes_Ecs"
include "Editor/vendor/sharedlib"
include "Editor/vendor/physx"
include "Editor/vendor/vulkan"
include "Editor/vendor/scripting"
include "Editor/vendor/scriptcore"
include "Editor/vendor/discord"
include "Editor/vendor/steam"
group ""

include "Editor"