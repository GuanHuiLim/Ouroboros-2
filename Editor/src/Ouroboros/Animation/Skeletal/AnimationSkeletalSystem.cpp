/************************************************************************************//*!
\file           AnimationSkeletalSystem.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          
Animation skeletal system enables skeletal animations to be updated and played
on game objects

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AnimationSkeletalSystem.h"
#include "AnimationSkeletalComponent.h"
#include "AnimationSkeletonStateMachine.h"
namespace oo::SkAnim
{
	void AnimationSkeletalSystem::Init(Ecs::ECSWorld* world, Scene* scene)
	{
	}
	void AnimationSkeletalSystem::BeforeUpdateLoop()
	{
	}

	void ApplyOutputPoseToSkeleton(AnimationSkeletalComponent& comp)
	{
		//TODO
	}

	void ApplySkeletonTransformToGameObjects(GameObject go, AnimationSkeletalComponent& comp)
	{
		//TODO
	}

	void AnimationSkeletalSystem::Run(Ecs::ECSWorld* world)
	{
		static Ecs::Query query = Ecs::make_query<AnimationSkeletalComponent, TransformComponent>();

		world->for_each_entity_and_component(query, [&](Ecs::EntityID entity, AnimationSkeletalComponent& comp) {
			GameObject go{ entity , *scene };

			internal::UpdateStateMachineProgress(go, comp);
			ApplyOutputPoseToSkeleton(comp);
			ApplySkeletonTransformToGameObjects(go, comp);
			});
	}
} //namespace oo::SkAnim
