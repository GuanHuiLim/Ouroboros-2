/************************************************************************************//*!
\file           PrefabManager.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          contain function to create prefab 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <unordered_map>
#include <Ouroboros/ECS/GameObject.h>
namespace oo
{

	class PrefabManager
	{
	public:
		PrefabManager();
		~PrefabManager();
		static void MakePrefab(std::shared_ptr<oo::GameObject> go);
		static void BreakPrefab(std::shared_ptr<oo::GameObject> go);
	private:

	};

};