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

    --platform defines
    filter{ "platforms:Executable"}
        defines "OO_EXECUTABLE"
    filter{}
    
    filter{ "platforms:Editor"}
        defines "OO_EDITOR"
    filter{}

    flags
    {
        "MultiProcessorCompile", -- enable multicore compilation
    }


group "Dependencies"
include "Editor/vendor/imgui"
include "Editor/vendor/launcher"
group ""

include "Editor"