/************************************************************************************//*!
\file           SceneInfo.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 31, 2022
\brief          Scene Info describes a basic struct of what's required when passing
                scenes for loading and changing scenes

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <string>

namespace oo
{
    class RuntimeScene;

    struct SceneInfo final
    {
        std::string SceneName;
        std::string LoadPath;

        SceneInfo(std::string_view name, std::string_view path) 
            : SceneName{ name }
            , LoadPath{ path }
        {}

        // Info is equivalent if they have the Same LoadPath.
        bool operator==(SceneInfo const& other) const
        {
            return LoadPath == other.LoadPath;
        }

        bool operator==(std::string_view path) const
        {
            return LoadPath == path;
        }
    };
}