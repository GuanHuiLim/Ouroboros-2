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
#include "AnimationInternal.h"

#include "Ouroboros/Core/Input.h"
#include "Ouroboros/Vulkan/MeshRendererComponent.h"
#include "Project.h"
namespace oo::Anim
{
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

		/*world->for_each(query, [&](oo::AnimationComponent& animationComp) {
			if (animationComp.GetAnimationTree() == nullptr)
			{
				LOG_CORE_DEBUG_CRITICAL("An animation component does not have an animation tree!!");
				assert(false);
				return;
			}

			internal::InitializeTracker(animationComp.actualComponent);
		});*/
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

		//TODO: replace 0.016f with delta time
		world->for_each_entity_and_component(query, [&](Ecs::EntityID entity, oo::AnimationComponent& animationComp) {
			GameObject go{ entity , *scene };
			internal::UpdateTrackerInfo info{ *this,animationComp.GetActualComponent(),animationComp.GetTracker(), entity,go.GetInstanceID(), 0.016f };
			internal::UpdateTracker(info);
			});

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



		world->for_each_entity_and_component(query, [&](Ecs::EntityID entity, oo::AnimationComponent& animationComp) {

			animationComp.Set_Root_Entity(entity);
			internal::InitialiseComponent(animationComp.GetActualComponent());

			});

		//set all condition's parameter index 
		for (auto& [key, tree] : AnimationTree::map)
		{
			internal::BindConditionsToParameters(tree);
			internal::BindNodesToAnimations(tree);
			internal::CalculateAnimationLength(tree);
			internal::ReloadReferences(tree);
		}
	}

	Scene::go_ptr AnimationSystem::CreateAnimationTestObject()
	{
		//MeshHierarchy::

		auto animationfbx_fp = Project::GetAssetFolder().string();
		//animationfbx_fp += "/AnimationTest_Character_IdleJumpAttack.fbx";

		//Animation::LoadAnimationFromFBX(animationfbx_fp,);

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
		auto start_node = tree->groups.begin()->second.startNode;

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
			.animation_name{ Animation::empty_animation_name },
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
		auto linkName = link->name;


		//add a condition to parameter
		ConditionInfo condition_info{
			.comparison{Condition::CompareType::LESS},
			.parameter_name{"Test float"},
			.value{20.f}
		};
		auto condition = comp.AddCondition(group.name, linkName, condition_info);
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
			.data{glm::vec3{0.f,0.f,0.f}},
			.time{0.f}
			};
			auto Keyframe1 = comp.AddKeyFrame(group.name, node->name, timeline->name, kf1);
			assert(Keyframe1);
			KeyFrame kf2{
				.data{glm::vec3{10.f,0.f,0.f}},
				.time{2.f}
			};
			auto Keyframe2 = comp.AddKeyFrame(group.name, node->name, timeline->name, kf2);
			assert(Keyframe2);
			KeyFrame kf3{
				.data{glm::vec3{10.f,10.f,0.f}},
				.time{4.f}
			};
			auto Keyframe3 = comp.AddKeyFrame(group.name, node->name, timeline->name, kf3);
			assert(Keyframe3);
			KeyFrame kf4{
				.data{glm::vec3{0.f,10.f,0.f}},
				.time{6.f}
			};
			auto Keyframe4 = comp.AddKeyFrame(group.name, node->name, timeline->name, kf4);
			assert(Keyframe4);
		}

		test_obj = obj;

		SaveAnimationTree("Test Animation Tree", animationfbx_fp + "/Test_Animation_Tree.tree");
		return obj;

	}
	bool AnimationSystem::SaveAnimationTree(std::string name, std::string filepath)
	{
		//map should contain the animation tree
		assert(AnimationTree::map.contains(name));

		auto& tree = AnimationTree::map[name];
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
		//writer.StartObject();
		//{
		//	writer.Key("AnimationTree");
		//	writer.String(tree.name.c_str(), static_cast<rapidjson::SizeType>(tree.name.size()));
		//	/*------------------
		//	PARAMETERS
		//	------------------*/
		//	writer.Key("Parameters");
		//	{
		//		writer.StartArray();
		//		for (auto& param : tree.parameters)
		//		{
		//			writer.StartArray();
		//			writer.String(param.name.c_str(), static_cast<rapidjson::SizeType>(param.name.size()));
		//			Parameter::serializeFn_map.at(param.type)(writer, param);
		//			writer.EndArray();

		//		}
		//		writer.EndArray();
		//	}
		//	/*------------------
		//	GROUPS
		//	------------------*/
		//	writer.Key("Groups");
		//	{
		//		writer.StartArray();
		//		for (auto& [uid, group] : tree.groups)
		//		{
		//			Group::serializeFn(writer, group);
		//		}
		//		writer.EndArray();
		//	}
		//}
	}
	
}
