-- Dependencies throughout project
App                         = "Editor"

MainDir                     = "%{wks.location}"
AppDir                      = MainDir .. "/" .. App

AppVendor                   = AppDir .. "/vendor"

-- where the files are output to
outputdir = "%{cfg.buildcfg}-%{cfg.platform}-%{cfg.system}-%{cfg.architecture}"
-- binaries location shortcuts
binOut                      = "../bin/" .. outputdir
binApp                      = binOut .. "/" .. App

-- -- Include directories relative to root folder (solution directory)
-- IncludeDir                  = {}
-- IncludeDir["glad"]          = EngineVendor .. "/glad/include"
-- IncludeDir["glm"]           = EngineVendor .. "/glm"
-- IncludeDir["oom"]           = EngineVendor .. "/oom"   -- custom maths library
-- IncludeDir["ImGui"]         = EngineVendor .. "/ImGui"
-- IncludeDir["rttr"]          = EngineVendor .. "/rttr/include"
-- IncludeDir["SDL"]           = EngineVendor .. "/sdl2/include"
-- IncludeDir["spdlog"]        = EngineVendor .. "/spdlog/include"
-- IncludeDir["stb_image"]     = EngineVendor .. "/stb_image"
-- IncludeDir["fmod"]          = EngineVendor .. "/fmod/core/inc"
-- IncludeDir["fmod_studio"]   = EngineVendor .. "/fmod/studio/inc"    -- fmod studio requires fmod inc too
-- IncludeDir["ffmpeg"]      	= EngineVendor .. "/ffmpeg/"
-- IncludeDir["freetype"]      = EngineVendor .. "/freetype/include"
-- IncludeDir["mono"]          = EngineVendor .. "/mono/include/mono-2.0"
-- IncludeDir["tracy"]         = EngineVendor .. "/tracy"
-- IncludeDir["rapidjson"]     = EngineVendor .. "/rpj"
-- -- Editor include dependencies

-- -- Paths to various external libraries directories
-- LibraryDir                  = {}
-- LibraryDir["mono"]          = EngineVendor .. "/mono/lib"
-- LibraryDir["rttr"]          = EngineVendor .. "/rttr/lib"
-- LibraryDir["SDL"]           = EngineVendor .. "/sdl2/lib/x64"
-- LibraryDir["fmod"]          = EngineVendor .. "/fmod/core/lib/x64"
-- LibraryDir["fmod_studio"]   = EngineVendor .. "/fmod/studio/lib/x64"
-- LibraryDir["ffmpeg"]   		= EngineVendor .. "/ffmpeg/lib64"
-- LibraryDir["freetype"]      = EngineVendor .. "/freetype/x64"
-- LibraryDir["oom"]           = EngineVendor .. "/oom/lib"   -- custom maths library directory
-- Editor Library dependencies