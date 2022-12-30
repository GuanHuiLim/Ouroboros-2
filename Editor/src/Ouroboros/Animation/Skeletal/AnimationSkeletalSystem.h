/************************************************************************************//*!
\file           AnimationSkeletalSystem.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
Animation skeletal system enables skeletal animations to be updated and played
on game objects

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "../Anim_Utils.h"
#include "AnimationSkeletalComponent.h"

#include "Ouroboros/ECS/ArchtypeECS/Wrapper.h"
#include "Ouroboros/ECS/ArchtypeECS/System.h"
#include "App/Editor/Events/OpenFileEvent.h"
#include "App/Editor/Events/LoadProjectEvents.h"
namespace oo::Anim
{
	class AnimationSkeletalSystem : public Ecs::System
	{
		Ecs::ECSWorld* world{ nullptr };
		Scene* scene{ nullptr };
		Scene::go_ptr test_obj{};
	public:
		AnimationSkeletalSystem() = default;
		~AnimationSkeletalSystem();
		//to be run before main gameplay loop
		void Init(Ecs::ECSWorld* world, Scene* scene);
		//to be run before main gameplay loop and after objects are created/loaded
		void BeforeUpdateLoop();
		//update function to be run every frame
		void Run(Ecs::ECSWorld* world) override;
	private:
	};


	

	
}
