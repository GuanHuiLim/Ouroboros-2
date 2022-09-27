project "Physics"
    kind "StaticLib"
    language "C++"
    staticruntime "off"
    warnings "Extra" -- Set warnings level to 4 for this project

    -- output directory
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Physics/Source/*.*"
    }

    includedirs
    {
        "%{IncludeDir.physx}",
        --"%{IncludeDir.physx}/Physics/Physx/pxshared/include",
        "%{IncludeDir.physx_foundation}"
    }
    
    links
    {
        "PhysX_64",
        "PhysXCommon_64",
        "PhysXCooking_64",
        "PhysXFoundation_64",
        "PhysXExtensions_static_64",
        "PhysXPvdSDK_static_64",
    }

    -- Physics Level Disable Linker Warning 
    linkoptions 
    { 
        "-IGNORE:4006", -- redefinition <- appropriate warning should look into it
        "-IGNORE:4099", -- physXPVD .PDB not found <- normal as we are not using it in our editor.
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
        defines "_DEBUG"
        
        -- library diretories
        -- CLIENT : Only Add client required Directories, rest settled by references
        libdirs 
        {
            "%{LibraryDir.physx}/Debug",
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        defines "NDEBUG"
        -- library diretories
        -- CLIENT : Only Add client required Directories, rest settled by references
        libdirs 
        {
            "%{LibraryDir.physx}/Release",
        }
        
    filter "configurations:Production"
        runtime "Release"
        optimize "on"
        defines "NDEBUG"
        -- library diretories
        -- CLIENT : Only Add client required Directories, rest settled by references
        libdirs 
        {
            "%{LibraryDir.physx}/Release",
        }