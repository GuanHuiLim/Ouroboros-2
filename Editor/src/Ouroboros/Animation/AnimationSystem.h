/************************************************************************************//*!
\file           AnimationSystem.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
Animation system enables animations to be updated and played
on game objects

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"

#include "Archetypes_Ecs/src/Wrapper.h"
#include "Archetypes_Ecs/src/System.h"
#include "App/Editor/Events/OpenFileEvent.h"
namespace oo::Anim
{
	class AnimationSystem : public Ecs::System
	{
		Ecs::ECSWorld* world{ nullptr };
		Scene* scene{ nullptr };
		Scene::go_ptr test_obj{};
	public:
		AnimationSystem() = default;
		~AnimationSystem();
		//to be run before main gameplay loop
		void Init(Ecs::ECSWorld* world, Scene* scene);
		//to be run before main gameplay loop and after objects are created
		void BindPhase();
		void Run(Ecs::ECSWorld* world) override;

		Ecs::ECSWorld* Get_Ecs_World()
		{
			return world;
		}
		Scene& Get_Scene()
		{
			assert(scene);
			return *scene;
		}

		Scene::go_ptr CreateAnimationTestObject();

		bool SaveAnimationTree(std::string name, std::string filepath);
		bool SaveAnimation(std::string name, std::string filepath);
		bool SaveAllAnimations(std::string filepath);
		bool LoadAnimationTree(std::string filepath);
		bool LoadAnimation(std::string filepath);
		bool LoadAssets(std::string filepath);
		void OpenFileCallback(OpenFileEvent* evnt);

		static Animation* AddAnimation(std::string const& name);
	private:
		bool SaveAnimation(Animation& anim, std::string filepath);
		void TestObject();
	};


	

	
}
