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

		Animation& operator*() const { return (*anims)[id]; }
		Animation* operator->() const { return &((*anims)[id]); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the animation in the animations vector
		void Reload();
		RTTR_ENABLE();
	private:
		bool valid() const;
	};

	

	
	
	

	struct KeyFrame
	{
		//position & scale vec3, rotation quat
		//using DataType = std::variant<glm::vec3,glm::quat,bool>;
		using DataType = rttr::variant;
		//vector3 variable HERE
		DataType data;
		float time{ 0.f };
	};

	struct ScriptEvent
	{
		//scriptevent handle or something variable HERE
		oo::ScriptValue::function_info script_function_info{};
		float time{0.f};
	};

	//stores a collection of Keyframes that edit a specific value in a component
	struct Timeline
	{
		enum class TYPE : int
		{
			PROPERTY,
			FBX_ANIM,
			SCRIPT_EVENT
		};

		enum class DATATYPE
		{
			VEC3,
			QUAT,
			BOOL,
		};
		std::string name{"Unnamed Timeline"};
		//what kind of timeline is it for, script events, or property, or fbx animation
		const TYPE type{};
		const DATATYPE datatype{};
		std::vector<int> children_index;
		std::vector<KeyFrame> keyframes;
		//function pointer to get the component
		Ecs::GetCompFn* get_componentFn{nullptr};
		rttr::type rttr_type;
		rttr::property rttr_property;
		size_t component_hash;

		//Timeline(TYPE _type, DATATYPE _datatype, std::string const _name = "Unnamed Timeline");
		Timeline(TimelineInfo const& info);
	};

	struct TimelineInfo
	{
		//type of animation, PROPERTY - user created, FBX_ANIM - loading from fbx file
		Timeline::TYPE type;
		//component hash of the ecs component that the timeline affects
		size_t component_hash;
		//property to set for the component
		rttr::property rttr_property;
		//property_name - optional
		std::string property_name{};
		//name of the timeline
		std::string timeline_name{ "Unnamed Timeline" };
		//uuid of the target gameobject to manipulate
		oo::GameObject target_object;
		//uuid of the gameobject the AnimationComponent is attached to
		oo::GameObject source_object;
		//do not touch
		std::vector<int> children_index{};
	};
	//tracks progress in 1 timeline
	struct ProgressTracker
	{
		using UpdateFn = void(*)(internal::UpdateProgressTrackerInfo&, float);

		Timeline::TYPE type{};
		size_t index{ 0ul };	//last index of whatever keyframe we were at
		UpdateFn updatefunction{ nullptr };
		Timeline* timeline{nullptr};

		ProgressTracker(const Timeline::TYPE _type);
		static ProgressTracker Create(Timeline::TYPE type);
	};

	struct Animation
	{
		static constexpr const char* empty_animation_name = "empty animation";
		static std::unordered_map< std::string, size_t> name_to_ID;
		static std::map<size_t, Animation> animation_storage;


		std::string name{ "Unnamed Animation" };

		std::vector<ScriptEvent> events{};

		std::vector<Timeline> timelines{};

		bool looping{ false };
		
		float animation_length{0.f};

		size_t animation_ID{ internal::generateUID() };

		static void LoadAnimationFromFBX(std::string const& filepath, ModelFileResource* resource);
		static Animation* AddAnimation(Animation& anim);

	};	

	

	
}




namespace oo
{
	class AnimationComponent;
}
