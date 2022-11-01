/************************************************************************************//*!
\file           Anim_Utils.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 25, 2022
\brief          Utility header for Animation.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <vector>
#include <variant>

#include "Archetypes_Ecs/src/EcsUtils.h"
#include "Ouroboros/EventSystem/EventSystem.h"
#include "Ouroboros/Scripting/ScriptValue.h"
#include "Ouroboros/Scripting/ScriptInfo.h"

#include <glm/gtc/quaternion.hpp>
#include <rttr/type.h>
#include <rttr/variant.h>
#include <rttr/property.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

namespace oo::Anim::internal
{
	constexpr uint expected_num_anims = 50;
	constexpr size_t invalid_ID{ std::numeric_limits<size_t>().max() };
	constexpr uint invalid_index{ std::numeric_limits<uint>().max() };
	extern std::unordered_map< size_t, rttr::instance(*)(void*)> hash_to_instance;

	void Initialise_hash_to_instance();

	size_t generateUID();
}

namespace oo::Anim
{
	enum class ParamType : int
	{
		BOOL,
		TRIGGER,
		INT,
		FLOAT,
	};
	using P_TYPE = ParamType;
	//using ParameterValueType = std::variant<bool, int, float>;

	//variables that are defined within an AnimationTree that can be accessed and assigned values from scripts or editor
	struct Parameter;	
	struct ParameterInfo;	//information to create a parameter
	struct Condition;		//checks against a parameter
	struct ConditionInfo;	//information to create a condition
	struct Link;			//connects two nodes with conditions
	struct LinkRef;
	struct KeyFrame;
	struct ScriptEvent;
	struct TimelineInfo;
	struct Timeline;
	struct TimelineRef;
	struct Animation;		//represents property or fbx file animations
	struct Node;			//a node in the animation tree
	struct NodeRef;			//a reference class to reference a node
	struct NodeInfo;
	struct Group;
	struct GroupInfo;
	struct GroupRef;
	struct AnimationTree; 
	struct AnimationTracker; //tracks a user's progress in an animation tree
	struct ProgressTracker;
	class IAnimationComponent;
	class AnimationSystem;

	namespace internal
	{
		struct UpdateTrackerInfo
		{
			AnimationSystem& system;
			IAnimationComponent& comp;
			AnimationTracker& tracker;
			Ecs::EntityID entity;
			oo::UUID uuid;
			float dt;
		};
		//info for a single timeline's progress
		struct UpdateProgressTrackerInfo
		{
			UpdateTrackerInfo& tracker_info;
			ProgressTracker& progressTracker;
		};

		constexpr char  serialize_method_name[] = "Serialize";
		constexpr char  load_method_name[] = "Load";
	}

	struct GroupRef
	{
		std::map<size_t, Group>* groups{ nullptr }; //reference to container of groups
		//int index{ -1 };	//group index
		size_t id{ internal::invalid_ID }; //group's unique identifier

		GroupRef(std::map<size_t, Group>* _groups = nullptr, size_t _id = { internal::invalid_ID });
		Group& operator*() const { return Retrieve(id); }
		Group* operator->() const { return &Retrieve(id); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the group in the groups vector
		void Reload();
		RTTR_ENABLE();
	private:
		Group& Retrieve(size_t id) const;
		bool valid() const;
	};

	struct NodeRef
	{
		std::map<size_t, Node>* nodes{ nullptr }; //reference to container of nodes
		//int index{ -1 };	//node index
		size_t id{ internal::invalid_ID }; //node's unique identifier

		NodeRef(std::map<size_t, Node>* _nodes = nullptr, size_t _id = { internal::invalid_ID });
		Node& operator*() const { return Retrieve(id); }
		Node* operator->() const { return &Retrieve(id); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the node in the nodes vector
		void Reload();
		RTTR_ENABLE();
	private:
		Node& Retrieve(size_t id) const;
		bool valid() const;

		
	};

	struct LinkRef
	{
		std::map<size_t, Link>* links{ nullptr }; //reference to container of groups
		//int index{ -1 };	//link index
		size_t id{ internal::invalid_ID }; //link's unique identifier

		LinkRef(std::map<size_t, Link>* _links = nullptr, size_t _id = { internal::invalid_ID });
		Link& operator*() const { return Retrieve(id); }
		Link* operator->() const { return &Retrieve(id); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the link in the links vector
		void Reload();
		RTTR_ENABLE();
	private:
		Link& Retrieve(size_t id) const;
		bool valid() const;
	};

	struct AnimRef
	{
		std::map<size_t, Animation>* anims{ nullptr }; //reference to container of animations
		//int index{ -1 };	//animation index
		size_t id{ internal::invalid_ID }; //animation's unique identifier

		Animation& operator*() const { return Retrieve(id); }
		Animation* operator->() const { return &Retrieve(id); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the animation in the animations vector
		void Reload();
		RTTR_ENABLE();
	private:
		Animation& Retrieve(size_t id) const;
		bool valid() const;
	};


	

	
	
	
}




namespace oo
{
	class AnimationComponent;
}
