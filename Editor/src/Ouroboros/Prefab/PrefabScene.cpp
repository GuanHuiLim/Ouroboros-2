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
//#include "App/Editor/Serializer.h"
#include <fstream>
namespace oo
{
    PrefabScene::PrefabScene(std::string const& filepath)
        : Scene{ "Prefab Scene (For Instancing)" }
    {
    }

	std::string& PrefabScene::GetPrefab(std::string const& filepath)
	{
		return LookUpPrefab(filepath);
	}

	std::string& PrefabScene::LoadPrefab(std::string const& filepath)
	{
		std::ifstream ifs(filepath);
		if (ifs.good())
		{
			std::stringstream buffer;
			buffer << ifs.rdbuf();
			m_loadedPrefabMap.emplace(filepath, buffer.str());
		}
		ifs.close();
		return m_loadedPrefabMap[filepath];
	}

	std::string& PrefabScene::LookUpPrefab(std::string const& filepath)
	{
		auto iter = m_loadedPrefabMap.find(filepath);
		if (iter == m_loadedPrefabMap.end())
			return LoadPrefab(filepath);
		return iter->second;
	}

    /*Scene::go_ptr PrefabScene::GetPrefab(std::string const& filepath)
    {
		Scene::go_ptr prefab = LookUpPrefab(filepath);
		if (prefab == nullptr)
			prefab = LoadPrefab(filepath);
        return prefab;
    }

    bool PrefabScene::PrefabIsLoaded(std::string const& filepath) const
    {
		auto iter = m_loadedPrefabMap.find(filepath);
        return iter != m_loadedPrefabMap.end();
    }

    Scene::go_ptr PrefabScene::LoadPrefab(std::string const& filepath)
    {
		UUID prefab = Serializer::LoadPrefab(filepath, this->GetRoot(), *this);
		Scene::go_ptr gameobject = this->FindWithInstanceID(prefab);

		ASSERT_MSG(gameobject == nullptr, "GAMEOBJECT IS NULL WHY?");
		m_loadedPrefabMap.emplace(filepath, gameobject);
		return gameobject;
    }

    Scene::go_ptr PrefabScene::LookUpPrefab(std::string const& filepath)
    {
		auto iter = m_loadedPrefabMap.find(filepath);
		if (iter == m_loadedPrefabMap.end())
			return nullptr;

        return iter->second;
    }*/

}