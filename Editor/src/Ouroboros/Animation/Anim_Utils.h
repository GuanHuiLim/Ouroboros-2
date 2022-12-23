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

#include "Ouroboros/ECS/ArchtypeECS/EcsUtils.h"
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
	auto constexpr EPSILON = std::numeric_limits < float > ::epsilon();
	//constexpr uint expected_num_anims = 50;
	constexpr size_t invalid_ID{ std::numeric_limits<size_t>().max() };
	constexpr uint invalid_index{ std::numeric_limits<uint>().max() };
	extern std::unordered_map< size_t, rttr::instance(*)(void*)> hash_to_instance;

	void Initialise_hash_to_instance();

	size_t generateUID();
	bool Equal(float lhs, float rhs);
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

	struct UID
	{
		size_t id{};
		UID() = default;
		operator size_t() const { return id; }
		UID(size_t _id) : id{_id}{}
	};

	struct InvalidType {};
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
	struct ScriptEventTracker;
	class IAnimationComponent;
	class AnimationSystem;
	class Pose; //represents a hierarchy of bones of skeletal mesh transforms
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

		struct UpdateScriptEventInfo
		{
			UpdateTrackerInfo& tracker_info;
			std::vector<ScriptEvent>& events;
		};

		constexpr char  serialize_method_name[] = "Serialize";
		constexpr char  load_method_name[] = "Load";
		constexpr char	start_node_name[] = "Start Node";
		constexpr char	any_state_node_name[] = "Any State Node";

		static constexpr const char* empty_animation_name = "empty animation";
		static inline UID empty_animation_UID{ 0ull };
	}

	struct GroupRef
	{
		std::map<size_t, Group>* groups{ nullptr }; //reference to container of groups
		//int index{ -1 };	//group index
		size_t id{ internal::invalid_ID }; //group's unique identifier

		GroupRef(std::map<size_t, Group>* _groups = nullptr, size_t _id = { internal::invalid_ID });
		Group& operator*() const { return Retrieve(); }
		Group* operator->() const { return &Retrieve(); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the group in the groups vector
		void Reload();
		RTTR_ENABLE();
	private:
		Group& Retrieve() const;
		bool valid() const;
	};

	struct NodeRef
	{
		std::map<size_t, Node>* nodes{ nullptr }; //reference to container of nodes
		//int index{ -1 };	//node index
		size_t id{ internal::invalid_ID }; //node's unique identifier

		NodeRef(std::map<size_t, Node>* _nodes = nullptr, size_t _id = { internal::invalid_ID });
		Node& operator*() const { return Retrieve(); }
		Node* operator->() const { return &Retrieve(); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the node in the nodes vector
		void Reload();
		RTTR_ENABLE();
	private:
		Node& Retrieve() const;
		bool valid() const;

		
	};

	struct LinkRef
	{
		std::map<size_t, Link>* links{ nullptr }; //reference to container of groups
		//int index{ -1 };	//link index
		size_t id{ internal::invalid_ID }; //link's unique identifier

		LinkRef(std::map<size_t, Link>* _links = nullptr, size_t _id = { internal::invalid_ID });
		Link& operator*() const { return Retrieve(); }
		Link* operator->() const { return &Retrieve(); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the link in the links vector
		void Reload();
		RTTR_ENABLE();
	private:
		Link& Retrieve() const;
		bool valid() const;
	};

	struct AnimRef
	{
		std::map<size_t, Animation>* anims{ nullptr }; //reference to container of animations
		//int index{ -1 };	//animation index
		size_t id{ internal::invalid_ID }; //animation's unique identifier

		AnimRef(std::map<size_t, Animation>* _anims = nullptr, size_t _id = { internal::invalid_ID });
		Animation& operator*() const { return Retrieve(); }
		Animation* operator->() const { return &Retrieve(); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the animation in the animations vector
		void Reload();
		RTTR_ENABLE();
	private:
		Animation& Retrieve() const;
		bool valid() const;
	};

	struct TimelineRef
	{
		AnimRef anim{};
		int index{-1};

		Timeline& operator*() const { return Retrieve(); }
		Timeline* operator->() const { return &Retrieve(); }
		operator bool() const {
			return valid();
		}
	private:
		Timeline& Retrieve() const;
		bool valid() const;
	};
	

	/*------------
	Info structs
	------------*/
	struct SetNodeAnimInfo
	{
		// name to identify group
		std::string group_name;
		// id of the node
		size_t node_ID{ internal::invalid_ID };
		// asset for the animation
		oo::Asset anim_asset;
	};
	struct TargetTimelineInfo
	{
		//id of the animation the timeline is located at
		size_t anim_id{internal::invalid_ID};
		//name of the timeline
		std::string timeline_name{};

	};
	struct TargetNodeInfo
	{
		//name of the group
		std::string group_name{};
		//id of the node
		size_t node_ID{internal::invalid_ID};
	};
	struct TargetLinkInfo
	{
		//name of the group
		std::string group_name{};
		//id of the link
		size_t link_ID{ internal::invalid_ID };
	};

	struct TargetConditionInfo
	{
		//information of the link the condition is attached to
		TargetLinkInfo link_info{};
		//id of the condition
		size_t condition_ID{ internal::invalid_ID };
	};

	struct TargetParameterInfo
	{
		//id of the parameter
		size_t param_ID{ internal::invalid_ID };
	};

	struct SplitAnimationInfo
	{
		bool in_frames{ false };

		//if in_frames is set to true use this
		size_t start_frame{ 0ull };
		size_t end_frame{ 0ull };

		//if in_frames is set to false use this
		float start_time{ 0.f };
		float end_time{ 0.f };
		//from animation->animation_ID 
		UID anim_ID{internal::invalid_ID};
		//desired split_animation_name
		std::string split_animation_name{};

		//leave this as empty to store in same directory as original animation
		std::string filepath{};
	};
}




namespace oo
{
	class AnimationComponent;
}
