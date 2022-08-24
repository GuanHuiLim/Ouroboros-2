#pragma once

#include <string>

namespace oo
{
    class RuntimeScene;

    struct SceneInfo final
    {
        std::string SceneName;
        std::string LoadPath;

        SceneInfo(std::string_view name, std::string_view path/*, std::size_t index*/) : SceneName{ name }, LoadPath{ path }{}
    };
}