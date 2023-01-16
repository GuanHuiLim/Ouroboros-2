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
IncludeDir["vulkan"]        = AppVendor .. "/vulkan"
IncludeDir["vulkanSrc"]     = AppVendor .. "/vulkan/OO_Vulkan/src"
IncludeDir["assimp"]        = AppVendor .. "/vulkan/vendor/assimp/include/"
IncludeDir["assimpBin"]     = AppVendor .. "/vulkan/vendor/assimp/BINARIES/Win32/include/"
IncludeDir["rttr"]          = AppVendor .. "/rttr/src"
IncludeDir["SDL"]           = AppVendor .. "/sdl2/include"
IncludeDir["spdlog"]        = AppVendor .. "/spdlog/include"
-- IncludeDir["stb_image"]     = EngineVendor .. "/stb_image"
IncludeDir["fmod"]          = AppVendor .. "/fmod/core/inc"
-- IncludeDir["fmod_studio"]   = EngineVendor .. "/fmod/studio/inc"    -- fmod studio requires fmod inc too
-- IncludeDir["ffmpeg"]      	= EngineVendor .. "/ffmpeg/"
-- IncludeDir["freetype"]      = EngineVendor .. "/freetype/include"
IncludeDir["mono"]          =  AppVendor .. "/scripting/vendor/mono/include/mono-2.0"
IncludeDir["tracy"]         = AppVendor .. "/tracy"
IncludeDir["optick"]         = AppVendor .. "/optick"
IncludeDir["rapidjson"]     = AppVendor .. "/rapidjson/include"
IncludeDir["VulkanSDK"]     = "%{VULKAN_SDK}/Include"
-- IncludeDir["tinyobjloader"] = EngineVendor .. "/tinyobjloader"
-- IncludeDir["vkbootstrap"]   = EngineVendor .. "/vkbootstrap"
-- IncludeDir["vma"]           = EngineVendor .. "/vma"

IncludeDir["physx"]                 = AppVendor .. "/physx"
IncludeDir["physx_foundation"]      = AppVendor .. "/physx/Physics/Physx/pxshared/include"

IncludeDir["discord"]      = AppVendor .. "/discord/cpp"
IncludeDir["slikenet"]      = AppVendor .. "/slikenet/include"

-- Our External Submodules 
IncludeDir["launcher"]      = AppVendor .. "/launcher/Oroborous-Launcher"
--IncludeDir["ecs"]           = AppVendor .. "/Archetypes_Ecs"
IncludeDir["scripting"]     = AppVendor .. "/scripting/src"

IncludeDir["sharedlib"]     = AppVendor .. "/sharedlib/Isolated-Testing-Ground"

-- Paths to various external libraries directories
LibraryDir                  = {}
-- LibraryDir["mono"]          = EngineVendor .. "/mono/lib"
LibraryDir["rttr"]          = AppVendor .. "/rttr/lib"
LibraryDir["SDL"]           = AppVendor .. "/sdl2/lib/x64"
LibraryDir["physx"]           = AppVendor .. "/physx/Physics/Physx/lib"
-- LibraryDir["fmod"]          = EngineVendor .. "/fmod/core/lib/x64"
LibraryDir["fmod"]          = AppVendor .. "/fmod/core/lib/x64"
-- LibraryDir["fmod_studio"]   = EngineVendor .. "/fmod/studio/lib/x64"
-- LibraryDir["ffmpeg"]   		= EngineVendor .. "/ffmpeg/lib64"
-- LibraryDir["freetype"]      = EngineVendor .. "/freetype/x64"
-- LibraryDir["oom"]           = EngineVendor .. "/oom/lib"   -- custom maths library directory
LibraryDir["VulkanSDK"]     = "%{VULKAN_SDK}/Lib"
LibraryDir["assimp"]     	= AppVendor .. "/vulkan/vendor/assimp/BINARIES/Win32/lib"
-- LibraryDir["tinyobjloader"] = EngineVendor .. "/tinyobjloader/lib"
-- LibraryDir["vkbootstrap"]   = EngineVendor .. "/vkbootstrap/lib"
-- LibraryDir["launcher"]      = AppVendor .. "launcher/Oroborous-Launcher/lib"
LibraryDir["discord"]       = AppVendor .. "/discord/lib/x86_64"
LibraryDir["slikenet"]       = AppVendor .. "/slikenet/lib/"

-- Paths to libraries that will be used
Library                     = {}
Library["Vulkan"]           = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
