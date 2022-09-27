project "ImGui"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    -- output directory
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        -- "imconfig.h",
        -- "imgui.h",
        -- "imgui.cpp",
        -- "imgui_draw.cpp",
        -- "imgui_internal.h",
		-- "imgui_stdlib.h",
		-- "imgui_stdlib.cpp",
        -- "imgui_widgets.cpp",
        -- "imstb_rectpack.h",
        -- "imstb_textedit.h",
        -- "imstb_truetype.h",
        -- "imgui_demo.cpp",
        -- "imgui_tables.cpp",
        -- "imgui_impl_opengl3.h",
        -- "imgui_impl_opengl3.cpp",
        -- "imgui_impl_sdl.h",
        -- "imgui_impl_sdl.cpp",
        -- "ImGuizmo.h",
        -- "ImGuizmo.cpp",
        
        -- "examples/example_sdl_vulkan/**.h",
        -- "examples/example_sdl_vulkan/**.cpp",

        "imgui/backends/imgui_impl_vulkan.h",
        "imgui/backends/imgui_impl_vulkan.cpp",
        "imgui/backends/imgui_impl_sdl.h",
        "imgui/backends/imgui_impl_sdl.cpp",
        "imgui/backends/imgui_impl_win32.h",
        "imgui/backends/imgui_impl_win32.cpp",
        "imgui/imgui.h",
        "imgui/imgui.cpp",
        "imgui/imgui_demo.cpp",
        "imgui/imgui_draw.cpp",
        "imgui/imgui_internal.h",
        --"imgui_stdlib.h",
		--"imgui_stdlib.cpp",
        "imgui/imgui_widgets.cpp",
        "imgui/imstb_rectpack.h",
        "imgui/imstb_textedit.h",
        "imgui/imstb_truetype.h",
        "imgui/imgui_demo.cpp",
        "imgui/imgui_tables.cpp",
        "imgui/misc/cpp/**.cpp",
        "imgui/misc/cpp/**.h",
        
        -- ImGuizmo
        "imgui/ImGuizmo.h",
        "imgui/ImGuizmo.cpp",

        -- ImGui-Node-Editor
        "imgui/crude_json.h",
        "imgui/crude_json.cpp",
        "imgui/imgui_node_editor.h",
        "imgui/imgui_node_editor.cpp",
        "imgui/imgui_node_editor_api.cpp",
        "imgui/imgui_node_editor_internal.h",
        "imgui/imgui_canvas.h",
        "imgui/imgui_canvas.cpp",
        "imgui/imgui_bezier_math.h",
        "imgui/imgui_extra_math.h"
    }

    includedirs
    {
        "",
        "backends",
        "misc/cpp",
        --"%{IncludeDir.glad}",
        "%{IncludeDir.SDL}/sdl2",
        "%{IncludeDir.VulkanSDK}",
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
        defines "NDEBUG"