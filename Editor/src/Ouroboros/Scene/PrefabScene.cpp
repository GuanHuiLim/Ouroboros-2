/************************************************************************************//*!
\file           PrefabScene.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2022
\brief          PrefabScene describes the scene when its meant for editing.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "PrefabScene.h"
#include <filesystem>

namespace oo
{
    PrefabScene::PrefabScene(std::string const& filepath)
        : Scene{ "Prefab Scene (For Instancing)" }
    {
    }

    Scene::go_ptr PrefabScene::GetPrefab(std::string const& filepath)
    {
        if (PrefabIsLoaded(filepath) == false)
        {
            LoadPrefab(filepath);
        }
        
        return LookUpPrefab(filepath);
    }

    bool PrefabScene::PrefabIsLoaded(std::string const& filepath) const
    {
        return false;
    }

    void PrefabScene::LoadPrefab(std::string const& filepath)
    {
    }

    Scene::go_ptr PrefabScene::LookUpPrefab(std::string const& filepath)
    {
    }

}