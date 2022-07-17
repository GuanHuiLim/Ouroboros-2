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

-- retrieving vulkan from pc
VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include directories relative to root folder (solution directory)
IncludeDir                  = {}
-- IncludeDir["glad"]          = EngineVendor .. "/glad/include"
IncludeDir["glm"]           = AppVendor .. "/glm"
-- IncludeDir["oom"]           = EngineVendor .. "/oom"   -- custom maths library
IncludeDir["imgui"]         = AppVendor .. "/imgui"
-- IncludeDir["rttr"]          = EngineVendor .. "/rttr/include"
IncludeDir["SDL"]           = AppVendor .. "/sdl2/include"
IncludeDir["spdlog"]        = AppVendor .. "/spdlog/include"
-- IncludeDir["stb_image"]     = EngineVendor .. "/stb_image"
-- IncludeDir["fmod"]          = EngineVendor .. "/fmod/core/inc"
-- IncludeDir["fmod_studio"]   = EngineVendor .. "/fmod/studio/inc"    -- fmod studio requires fmod inc too
-- IncludeDir["ffmpeg"]      	= EngineVendor .. "/ffmpeg/"
-- IncludeDir["freetype"]      = EngineVendor .. "/freetype/include"
-- IncludeDir["mono"]          = EngineVendor .. "/mono/include/mono-2.0"
-- IncludeDir["tracy"]         = EngineVendor .. "/tracy"
IncludeDir["rapidjson"]     = AppVendor .. "/rapidjson/include"
IncludeDir["VulkanSDK"]     = "%{VULKAN_SDK}/Include"
-- IncludeDir["tinyobjloader"] = EngineVendor .. "/tinyobjloader"
-- IncludeDir["vkbootstrap"]   = EngineVendor .. "/vkbootstrap"
-- IncludeDir["vma"]           = EngineVendor .. "/vma"

-- Our External Submodules 
IncludeDir["launcher"]      = AppVendor .. "/launcher/Oroborous-Launcher"
IncludeDir["ecs"]           = AppVendor .. "/Archetypes_Ecs"

IncludeDir["sharedlib"]     = AppVendor .. "/sharedlib/Isolated-Testing-Ground"
--IncludeDir["quaternion"]    = AppVendor .. "/sharedlib/Isolated-Testing-Ground/Quaternion/include" 
--IncludeDir["scenegraph"]    = AppVendor .. "/sharedlib/Isolated-Testing-Ground/Scenegraph/include" 
--IncludeDir["scene"]         = AppVendor .. "/sharedlib/Isolated-Testing-Ground/SceneManagement/include" 


-- Paths to various external libraries directories
LibraryDir                  = {}
-- LibraryDir["mono"]          = EngineVendor .. "/mono/lib"
-- LibraryDir["rttr"]          = EngineVendor .. "/rttr/lib"
LibraryDir["SDL"]           = AppVendor .. "/sdl2/lib/x64"
-- LibraryDir["fmod"]          = EngineVendor .. "/fmod/core/lib/x64"
-- LibraryDir["fmod_studio"]   = EngineVendor .. "/fmod/studio/lib/x64"
-- LibraryDir["ffmpeg"]   		= EngineVendor .. "/ffmpeg/lib64"
-- LibraryDir["freetype"]      = EngineVendor .. "/freetype/x64"
-- LibraryDir["oom"]           = EngineVendor .. "/oom/lib"   -- custom maths library directory
LibraryDir["VulkanSDK"]     = "%{VULKAN_SDK}/Lib"
-- LibraryDir["tinyobjloader"] = EngineVendor .. "/tinyobjloader/lib"
-- LibraryDir["vkbootstrap"]   = EngineVendor .. "/vkbootstrap/lib"
-- LibraryDir["launcher"]      = AppVendor .. "launcher/Oroborous-Launcher/lib"

-- Paths to libraries that will be used
Library                     = {}
Library["Vulkan"]           = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
