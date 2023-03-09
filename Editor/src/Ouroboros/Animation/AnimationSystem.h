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


#include "Ouroboros/ECS/ArchtypeECS/Wrapper.h"
#include "Ouroboros/ECS/ArchtypeECS/System.h"
#include "App/Editor/Events/OpenFileEvent.h"
#include "App/Editor/Events/LoadProjectEvents.h"
namespace oo::Anim
{
	class AnimationSystem : public Ecs::System
	{
		Ecs::ECSWorld* world{ nullptr };
		Scene* scene{ nullptr };
		Scene::go_ptr test_obj{};

		static std::set<std::string> modified_animations;
		static std::set<std::string> modified_animation_trees;
	public:
		struct ModifyAnimationEvent : oo::Event {
			std::string name{};
		};
		struct ModifyAnimationTreeEvent : oo::Event {
			std::string name{};
		};

		AnimationSystem() = default;
		~AnimationSystem();
		//to be run before main gameplay loop
		void Init(Ecs::ECSWorld* world, Scene* scene);
		//to be run before main gameplay loop and after objects are created/loaded
		void BindPhase();
		//update function to be run every frame
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
		//test function
		Scene::go_ptr CreateAnimationTestObject();
		
		/*------------
		animation tree
		------------*/
		static bool SaveAnimationTree(size_t id, std::string filepath);
		//save all animation trees in respective asset filepath
		static bool SaveAllAnimationTree();
		//save all modified animation trees in respective asset filepath
		static bool SaveAllModifiedAnimationTree();
		static void NotifyModifiedAnimationTree(std::string const tree_name);
		
		//save all animation trees in filepath
		static bool SaveAllAnimationTree(std::string filepath);
		static AnimationTree* LoadAnimationTree(std::string filepath);
		static oo::Asset GetAnimationTreeAsset(UID anim_ID);
		/*---------
		animation
		---------*/
		//save all animations in respective asset filepath
		static bool SaveAllAnimations();
		//save all modified animations in respective asset filepath
		static bool SaveModifiedAnimations();
		static void NotifyModifiedAnimation(std::string const anim_name);
		//save all animations in filepath
		static bool SaveAllAnimations(std::string filepath);
		static bool SaveAnimation(std::string name, std::string filepath);
		static Animation* LoadAnimation(std::string filepath);
		static std::vector<Animation*> LoadAnimationFromFBX(std::string const& filepath, ModelFileResource* resource);
		static bool DeleteAnimation(std::string const& name);
		static bool SplitAnimation(SplitAnimationInfo& info);
		static oo::Asset GetAnimationAsset(UID anim_ID);

		static bool LoadAssets(std::string filepath);
		static void OpenFileCallback(OpenFileEvent* evnt);
		static void CloseProjectCallback(CloseProjectEvent* evnt);
		static void ModifyAnimationCallback(ModifyAnimationEvent* evnt);
		static void ModifyAnimationTreeCallback(ModifyAnimationTreeEvent* evnt);
		

		static Animation* AddAnimation(std::string const& name);
		static AnimationTree* CreateAnimationTree(std::string const& name);
	private:
		static bool SaveAnimation(Animation& anim, std::string filepath);
		static oo::Asset AddAnimationAsset(Animation&& anim, std::string const& filepath);
		static bool SplitAnimation(SplitAnimationInfo& info, Animation& anim);

		static bool SaveAnimationTree(AnimationTree& tree, std::string filepath);
		void TestObject();
		void TestDemoObject();
	};


	

	
}
