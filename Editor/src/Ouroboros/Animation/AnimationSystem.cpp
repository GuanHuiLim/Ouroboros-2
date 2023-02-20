/************************************************************************************//*!
\file           AnimationSystem.cp
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
#include "pch.h"
#include "AnimationSystem.h"
#include "AnimationComponent.h"
#include "AnimationTracker.h"
#include "AnimationParameter.h"
#include "AnimationTimeline.h"
#include "AnimationKeyFrame.h"
#include "AnimationInternal.h"

#include "Ouroboros/Core/Input.h"
#include "Ouroboros/Vulkan/MeshRendererComponent.h"
#include "Project.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "App/Editor/UI/Tools/MeshHierarchy.h"
#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "Ouroboros/Core/Timer.h"

#define DEBUG_ANIMATION false
namespace oo::Anim
{
	AnimationSystem::~AnimationSystem()
	{
		EventManager::Unsubscribe<OpenFileEvent>(&AnimationSystem::OpenFileCallback);
		EventManager::Unsubscribe<CloseProjectEvent>(&AnimationSystem::CloseProjectCallback);
	}
	void AnimationSystem::Init(Ecs::ECSWorld* _world, Scene* _scene)
	{
		static Ecs::Query query = []() {
			Ecs::Query _query;
			_query.with<AnimationComponent>().build();
			return _query;
		}();
		internal::Initialise_hash_to_instance();
		this->world = _world;
		this->scene = _scene;

		EventManager::Subscribe<OpenFileEvent>(&AnimationSystem::OpenFileCallback);
		EventManager::Subscribe<CloseProjectEvent>(&AnimationSystem::CloseProjectCallback);

		/*world->for_each(query, [&](oo::AnimationComponent& animationComp) {
			if (animationComp.GetAnimationTree() == nullptr)
			{
				LOG_CORE_DEBUG_CRITICAL("An animation component does not have an animation tree!!");
				assert(false);
				return;
			}

			internal::InitializeTracker(animationComp.actualComponent);
		});*/
		auto asset_location = Project::GetAssetFolder().string();
		LoadAssets(asset_location);
	}
	void AnimationSystem::Run(Ecs::ECSWorld* _world)
	{
		static Ecs::Query query = []() {
			Ecs::Query _query;
			_query.with<AnimationComponent>().build();
			return _query;
		}();

		//test object code
		if (test_obj)
		{
			if (input::IsKeyPressed(input::KeyCode::SPACE))
			{
				test_obj->GetComponent<AnimationComponent>()
					.SetParameter("Test float", 30.f);
			}
		}
		
		TRACY_PROFILE_SCOPE_NC(Animation_Update, 0x00E0E3);

		world->for_each_entity_and_component(query, [&](Ecs::EntityID entity, oo::AnimationComponent& animationComp) {
			GameObject go{ entity , *scene };
			internal::UpdateTrackerInfo info{ *this,animationComp.GetActualComponent(),animationComp.GetTracker(), entity,go.GetInstanceID(), timer::dt() };
			internal::UpdateTracker(info);
			});
		TRACY_PROFILE_SCOPE_END();
		/*world->for_each(query, [&](AnimationComponent& animationComp) {
			internal::UpdateTracker(*this, animationComp, animationComp.GetTracker(), 0.016f);
			});*/


	}
	//to be called ONCE after no more changes are made to the animation data
	//and before the main game loop
	void AnimationSystem::BindPhase()
	{
		static Ecs::Query query = []() {
			Ecs::Query _query;
			_query.with<AnimationComponent>().build();
			return _query;
		}();

		//set all animation data 
		for (auto& [key, tree] : AnimationTree::map)
		{
			internal::BindConditionsToParameters(tree);
			internal::BindNodesToAnimations(tree);
			internal::CalculateAnimationLength(tree);
			internal::ReloadReferences(tree);
		}

		world->for_each_entity_and_component(query, [&](Ecs::EntityID entity, oo::AnimationComponent& animationComp) {

			animationComp.Set_Root_Entity(entity);
			internal::InitialiseComponent(animationComp.GetActualComponent());

			});

		
	}

	void AnimationSystem::TestObject()
	{
		auto obj = scene->CreateGameObjectImmediate();
		obj->Name() = "AnimationTestObject";
		obj->AddComponent<MeshRendererComponent>();
		auto& comp = obj->AddComponent<oo::AnimationComponent>();
		comp.Set_Root_Entity(obj->GetEntity());

		oo::GameObject source = *(obj.get());
		oo::GameObject target;
		{
			auto child_obj = scene->CreateGameObjectImmediate();
			obj->AddChild(*(child_obj.get()));
			auto child_obj_2 = scene->CreateGameObjectImmediate();
			obj->AddChild(*(child_obj_2.get()));

			auto child_obj_1_3 = scene->CreateGameObjectImmediate();
			obj->AddChild(*(child_obj_1_3.get()));

			target = *(child_obj_1_3.get());
		}

		//create the animation tree asset
		auto tree = AnimationTree::Create("Test Animation Tree");
		comp.SetAnimationTree("Test Animation Tree");
		auto& start_node = tree->groups.begin()->second.startNode;

		//add some test parameters to the animation tree
		{
			ParameterInfo param_info{
			.name{"Test int"},
			.type{P_TYPE::INT},
			//optional
			.value{ 10 }
			};
			comp.AddParameter(param_info);

			ParameterInfo param_info2{
			.name{"Test trigger"},
			.type{P_TYPE::TRIGGER}
			};
			comp.AddParameter(param_info2);

			ParameterInfo param_info3{
			.name{"Test float"},
			.type{P_TYPE::FLOAT},
			//optional
			.value{ 10.f }
			};
			comp.AddParameter(param_info3);
		}

		//add a node to the first group
		auto& group = tree->groups.begin()->second;

		NodeInfo nodeinfo{
			.name{ "Test Node" },
			.animation_name{ internal::empty_animation_name },
			.speed{ 1.f },
			.position{0.f,0.f,0.f}
		};

		auto node = comp.AddNode(group.name, nodeinfo);
		assert(node);
		node->GetAnimation().looping = true;

		//add a link from the start node to the test node
		auto link = comp.AddLink(group.name, start_node->name, node->name);
		assert(link);
		link->has_exit_time = false;
		link->exit_time = 1.5f;
		auto& linkName = link->name;


		//add a condition to parameter
		ConditionInfo condition_info{
			.comparison{Condition::CompareType::LESS},
			.parameter_name{"Test float"},
			.value{20.f}
		};
		auto condition = comp.AddCondition(group.name, linkName, condition_info);
		(void)condition;
		assert(condition);

		//add a timeline to the node's animation
		TimelineInfo timeline_info{
		.type{Timeline::TYPE::PROPERTY},
		.component_hash{Ecs::ECSWorld::get_component_hash<TransformComponent>()},
		.rttr_property{rttr::type::get< TransformComponent>().get_property("Position")},
		.timeline_name{"Test Timeline"},
		.target_object{target},
		.source_object{source}
		};
		auto timeline = comp.AddTimeline(group.name, node->name, timeline_info);

		//adding test keyframes
		{
			KeyFrame kf1{
			glm::vec3{0.f,0.f,0.f},
			0.f
			};
			auto Keyframe1 = comp.AddKeyFrame(group.name, node->name, timeline->name, kf1);
			assert(Keyframe1);
			(void)Keyframe1;
			KeyFrame kf2{
				glm::vec3{10.f,0.f,0.f},
				2.f
			};
			auto Keyframe2 = comp.AddKeyFrame(group.name, node->name, timeline->name, kf2);
			(void)Keyframe2;
			assert(Keyframe2);
			KeyFrame kf3{
				glm::vec3{10.f,10.f,0.f},
				4.f
			};
			auto Keyframe3 = comp.AddKeyFrame(group.name, node->name, timeline->name, kf3);
			assert(Keyframe3);
			(void)Keyframe3;
			KeyFrame kf4{
				glm::vec3{0.f,10.f,0.f},
				6.f
			};
			auto Keyframe4 = comp.AddKeyFrame(group.name, node->name, timeline->name, kf4);
			(void)Keyframe4;
			assert(Keyframe4);
		}

		test_obj = obj;

		auto animationfbx_fp = Project::GetAssetFolder().string();

		//searialization test for animation tree
		//SaveAnimationTree("Test Animation Tree", animationfbx_fp + "/Test_Animation_Tree.tree");		 
		//LoadAnimationTree(animationfbx_fp + "/Test_Animation_Tree.tree");


		//serialization test for animation
		/*{
			auto anim_name = node->GetAnimation().name;
			auto filepath = animationfbx_fp + "/" + anim_name + ".anim";
			SaveAnimation(node->GetAnimation(), filepath);
			LoadAnimation(filepath);

		}*/

	}

	void AnimationSystem::TestDemoObject()
	{
		auto anim_assets = Project::GetAssetManager()->GetAssetsByType(oo::AssetInfo::Type::Animation);
		oo::Asset animation_asset{};
		for (auto& asset : anim_assets)
		{
			auto name = asset.GetData<std::string>();
			if (name == "MainChar_Idle_Take 001")
			{
				animation_asset = asset;
				break;
			}
		}
		assert(animation_asset.IsValid());

		constexpr size_t treeID = 1234647030338911234ull;
		auto tree = internal::RetrieveAnimationTree("Demo Animation Tree");
		assert(tree);
		auto animation = internal::RetrieveAnimation("MainChar_Idle_Take 001");
		(void)animation;
		test_obj = scene->CreateGameObjectImmediate();
		test_obj->Name() = "AnimationTestObject";
		auto& comp = test_obj->AddComponent<oo::AnimationComponent>();
		comp.SetAnimationTree(tree->treeID);
		assert(comp.HasAnimationTree());

		//{//add conditions
		//	ConditionInfo condition_info{
		//	.comparison{Condition::CompareType::EQUAL},
		//	.parameter_name{"bool"},
		//	.value{true}
		//	};
		//	auto result = comp.AddCondition("Group 1", "Start Node -> Idle Node", condition_info);
		//	assert(result);
		//}
		//{//set animation
		//	SetNodeAnimInfo info{
		//	.group_name{"Group 1"},
		//	.node_ID{3935255677567460564ull},
		//	.anim_asset{ animation_asset },
		//	};
		//	auto result = comp.SetNodeAnimation(info);
		//	assert(result);
		//}
		
	}

	Scene::go_ptr AnimationSystem::CreateAnimationTestObject()
	{
		if constexpr (DEBUG_ANIMATION == false) return nullptr;

		else
		{
			/*auto animationfbx_fp = Project::GetAssetFolder().string();
			TestObject();
			auto obj_children = scene->GetRoot()->GetChildren();*/
			TestDemoObject();

			return nullptr;
		}
	}

	bool AnimationSystem::SaveAnimationTree(AnimationTree& tree, std::string filepath)
	{
		std::ofstream stream{ filepath ,std::ios::trunc };
		if (!stream)
		{
			assert(false);
			return false;
		}

		rapidjson::OStreamWrapper osw(stream);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

		//writer.Key("AnimationTree", static_cast<rapidjson::SizeType>(std::string("AnimationTree").size()));
		auto serialize_fn = rttr::type::get<AnimationTree>().get_method(internal::serialize_method_name);
		serialize_fn.invoke({}, writer, tree);

		assert(writer.IsComplete());
		stream.close();
		return true;
	}

	bool AnimationSystem::SaveAnimationTree(size_t id, std::string filepath)
	{

		//map should contain the animation tree
		assert(AnimationTree::map.contains(id));
		auto& tree = AnimationTree::map[id];

		return SaveAnimationTree(tree, filepath);
	}
	bool AnimationSystem::SaveAllAnimations()
	{
		for (auto& [id, anim] : Animation::animation_storage)
		{
			if (anim.animation_ID == internal::empty_animation_UID) continue;

			auto asset = GetAnimationAsset(id);
			if (asset.IsValid() == false)
			{
				assert(false);
				continue;
			}
			anim.name = asset.GetFilePath().stem().string();
			auto result = AnimationSystem::SaveAnimation(anim, asset.GetFilePath().string());
			if (result == false)
				return false;
		}
		return true;
	}

	bool AnimationSystem::SaveAllAnimations(std::string filepath)
	{
		for (auto& [id, anim] : Animation::animation_storage)
		{
			auto result = AnimationSystem::SaveAnimation(anim, filepath + "/" + anim.name + ".anim");
			if (result == false)
				return false;
		}
		return true;
	}

	bool AnimationSystem::SaveAllAnimationTree()
	{
		for (auto& [treeID, tree] : AnimationTree::map)
		{
			internal::CalculateAnimationLength(tree);
			internal::ReAssignReferences(tree);
			internal::ReloadReferences(tree);

			auto asset = GetAnimationTreeAsset(treeID);
			if (asset.IsValid() == false)
			{
				assert(false);
				continue;
			}
			tree.name = asset.GetFilePath().stem().string(); 
			auto result = AnimationSystem::SaveAnimationTree(tree, asset.GetFilePath().string());
			if (result == false)
				return false;
		}
		return true;
	}

	bool AnimationSystem::SaveAllAnimationTree(std::string filepath)
	{
		for (auto& [treeID, tree] : AnimationTree::map)
		{
			internal::CalculateAnimationLength(tree);
			internal::ReAssignReferences(tree);
			internal::ReloadReferences(tree);
			
			auto asset = GetAnimationTreeAsset(treeID);
			if (asset.IsValid() == false)
			{
				assert(false);
				continue;
			}
			tree.name = asset.GetFilePath().stem().string();
			auto result = AnimationSystem::SaveAnimationTree(tree, filepath + "/" + tree.name + ".tree");
			if (result == false)
				return false;
		}
		return true;
	}

	bool AnimationSystem::SaveAnimation(Animation& anim, std::string filepath)
	{
		//dont save empty animation
		if (anim.animation_ID == internal::empty_animation_UID) return true;

		std::ofstream stream{ filepath ,std::ios::trunc };
		if (!stream)
		{
			assert(false);
			return false;
		}
		anim.name = std::filesystem::path{ filepath }.stem().string();

		rapidjson::OStreamWrapper osw(stream);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		//writer.Key("AnimationTree", static_cast<rapidjson::SizeType>(std::string("AnimationTree").size()));
		auto serialize_fn = rttr::type::get<Animation>().get_method(internal::serialize_method_name);
		serialize_fn.invoke({}, writer, anim);

		assert(writer.IsComplete());
		stream.close();
		return true;
	}

	oo::Asset AnimationSystem::GetAnimationAsset(UID anim_ID)
	{
		auto assets = Project::GetAssetManager()->GetAssetsByType(oo::AssetInfo::Type::Animation);
		for (auto& asset : assets)
		{
			if (asset.GetData<UID>() == anim_ID)
				return asset;
		}
		return {};
	}

	oo::Asset AnimationSystem::GetAnimationTreeAsset(UID anim_ID)
	{
		auto assets = Project::GetAssetManager()->GetAssetsByType(oo::AssetInfo::Type::AnimationTree);
		for (auto& asset : assets)
		{
			if (asset.GetData<UID>() == anim_ID)
				return asset;
		}
		return {};
	}

	oo::Asset AnimationSystem::AddAnimationAsset(Animation&& anim, std::string const& filepath)
	{
		SaveAnimation(anim, filepath);
		return Project::GetAssetManager()->GetOrLoadPath(std::filesystem::path{ filepath });
	}

	bool AnimationSystem::SplitAnimation(SplitAnimationInfo& info, Animation& anim)
	{
		bool result{ false };
		auto new_anim = Animation::ExtractAnimation(info.start_time, info.end_time, anim, result);
		if (result == false)
		{
			assert(false);//extract animation failed
			return false;
		}
		new_anim.name = info.split_animation_name;
		new_anim.animation_length = (info.end_frame - info.start_frame) / new_anim.frames_per_second;

		auto existing = internal::RetrieveAnimation(new_anim.name);
		if (existing)
		{
			new_anim.animation_ID = existing->animation_ID;
			new_anim.looping = existing->looping;
			new_anim.events = existing->events;
			*existing = new_anim;
			return true;
		}
		//if uid is specified
		if (info.split_anim_ID != internal::invalid_ID)
			new_anim.animation_ID = info.split_anim_ID;
		auto filepath = info.filepath;
		//use original animation's filepath
		if (filepath.empty())
		{
			auto asset = GetAnimationAsset(info.anim_ID);
			if (asset.IsValid() == false)
			{
				assert(false);//original animation's asset doesn't exist
				return false;
			}

			filepath = asset.GetFilePath().parent_path().string() + "/" + new_anim.name + ".anim";
		}
		//add the animation as an asset
		auto asset = AddAnimationAsset(std::move(new_anim), filepath);
		if (asset.IsValid() == false)
		{
			assert(false); // animation asset invalid, did animation fail to load?
			return false;
		}

		return true;
	}

	bool AnimationSystem::SplitAnimation(SplitAnimationInfo& info)
	{
		
		auto anim_ptr = internal::RetrieveAnimation(info.anim_ID);
		if (anim_ptr == nullptr)
		{
			assert(false);//animation dont exist or not sotred
			return false;
		}
		if (info.split_animation_name.empty())
		{
			assert(false);//no name specified for split animation
			return false;
		}

		auto& anim = *anim_ptr;

		if (info.in_frames)
		{
			//if start_frame == end_frame do nothing
			if (info.start_frame == info.end_frame) return false;
			//convert to time
			info.start_time = anim.TimeFromFrame(info.start_frame);
			info.end_time = anim.TimeFromFrame(info.end_frame);
		}
		else
		{
			//if start_time == end_time do nothing
			if (internal::Equal(info.start_time, info.end_time))
				return false;
		}

		return SplitAnimation(info, anim);
	}


	bool AnimationSystem::SaveAnimation(std::string name, std::string filepath)
	{
		//map should contain the animation tree
		assert(Animation::name_to_ID.contains(name));
		assert(Animation::animation_storage.contains(Animation::name_to_ID[name]));

		auto& anim = Animation::animation_storage[Animation::name_to_ID[name]];
		return SaveAnimation(anim, filepath);
	}
	
	void AnimationSystem::OpenFileCallback(OpenFileEvent* evnt)
	{
		if (evnt->m_type != OpenFileEvent::FileType::FBX) return;

		//auto relativepath = std::filesystem::relative(evnt->m_filepath, Project::GetAssetFolder());
		//auto asset = Project::GetAssetManager()->GetOrLoadPath(relativepath);
		//auto resource = asset.GetData<ModelFileResource*>();

		//Animation::LoadAnimationFromFBX(evnt->m_filepath.string(), resource);
	}

	void AnimationSystem::CloseProjectCallback(CloseProjectEvent* evnt)
	{
		SaveAllAnimations();
		SaveAllAnimationTree();

	}

	bool AnimationSystem::LoadAssets(std::string filepath)
	{
		std::filesystem::path path{ filepath };

		for (auto& iter : path)
		{
			if (iter.extension().string() == ".tree")
			{
				auto result = LoadAnimationTree(iter.string());
				if (result == nullptr) return false;
			}
			if (iter.extension().string() == ".anim")
			{
				auto result = LoadAnimation(iter.string());
				if (result == nullptr) return false;
			}
		}
		return true;
	}

	AnimationTree* AnimationSystem::LoadAnimationTree(std::string filepath)
	{
		std::ifstream ifs;
		ifs.open(filepath);
		if (!ifs.is_open())
		{
			LOG_CRITICAL("LoadAnimationTree cannot find the file");
			assert(false);
			return nullptr;
		}
		rapidjson::IStreamWrapper isw(ifs);
		rapidjson::Document doc;
		doc.ParseStream(isw);

		auto obj = doc.GetObj();
		AnimationTree tree{};

		auto load_fn = rttr::type::get< AnimationTree>().get_method(internal::load_method_name);
		load_fn.invoke({}, obj, tree);
		tree.name = std::filesystem::path{ filepath }.stem().string();
		auto loaded_tree = AnimationTree::Add(std::move(tree));

		internal::ReAssignReferences(*loaded_tree);
		internal::ReloadReferences(*loaded_tree);

		return loaded_tree;
	}
	Animation* AnimationSystem::LoadAnimation(std::string filepath)
	{
		std::ifstream ifs;
		ifs.open(filepath);
		if (!ifs.is_open())
		{
			LOG_CRITICAL("LoadAnimationTree cannot find the file");
			assert(false);
			return nullptr;
		}
		rapidjson::IStreamWrapper isw(ifs);
		rapidjson::Document doc;
		doc.ParseStream(isw);
		
		Animation anim{};
		auto obj = doc.GetObj();
		auto load_fn = rttr::type::get< Animation>().get_method(internal::load_method_name);
		load_fn.invoke({}, obj, anim);
		anim.name = std::filesystem::path{ filepath }.stem().string();

		return Animation::AddAnimation(std::move(anim));

	}

	std::vector<Animation*> AnimationSystem::LoadAnimationFromFBX(std::string const& filepath, ModelFileResource* resource)
	{
		auto anims = Animation::LoadAnimationFromFBX(filepath, resource);

		for (auto& anim_ptr : anims)
		{
			if (anim_ptr == nullptr)
			{
				assert(false);
				continue;
			}
			auto anim_name = anim_ptr->name;
			auto anim_ID = anim_ptr->animation_ID;
			auto anim_filepath = std::filesystem::path{ filepath }.parent_path().string() + "/" + anim_ptr->name + ".anim";
			SaveAnimation(*anim_ptr, anim_filepath);
			DeleteAnimation(anim_name);
			/*auto asset = */Project::GetAssetManager()->GetOrLoadPath(std::filesystem::path{ anim_filepath });

			anim_ptr = internal::RetrieveAnimation(anim_ID);
		}


		return anims;
	}

	bool AnimationSystem::DeleteAnimation(std::string const& name)
	{
		Animation::RemoveAnimation(name);

		return true;
	}

	Animation* AnimationSystem::AddAnimation(std::string const& name)
	{		
		return internal::AddAnimationToStorage(name);
	}
	AnimationTree* AnimationSystem::CreateAnimationTree(std::string const& name)
	{
		auto tree = AnimationTree::Create(name);
		auto filepath = Project::GetAssetFolder().string() + "/" + tree->name + ".tree";
		SaveAnimationTree(*tree, filepath);
		auto asset = Project::GetAssetManager()->GetOrLoadPath(std::filesystem::path{ filepath });
		return tree;
	}
}
