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
	enum class ParamType
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
			float dt;
		};
		//info for a single timeline's progress
		struct UpdateProgressTrackerInfo
		{
			UpdateTrackerInfo& tracker_info;
			ProgressTracker& progressTracker;
		};
	}

	struct NodeRef
	{
		std::vector<Node>* nodes{ nullptr }; //reference to vector of nodes
		int index{ -1 };	//node index
		size_t id{ internal::invalid_ID }; //node's unique identifier

		Node& operator*() const { return (*nodes)[index]; }
		Node* operator->() const { return &((*nodes)[index]); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the node in the nodes vector
		void Reload();
	private:
		bool valid() const;
	};

	struct GroupRef
	{
		std::vector<Group>* groups{ nullptr }; //reference to vector of groups
		int index{ -1 };	//group index
		size_t id{ internal::invalid_ID }; //group's unique identifier

		Group& operator*() const { return (*groups)[index]; }
		Group* operator->() const { return &((*groups)[index]); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the group in the groups vector
		void Reload();
	private:
		bool valid() const;
	};

	struct LinkRef
	{
		std::vector<Link>* links{ nullptr }; //reference to vector of groups
		int index{ -1 };	//link index
		size_t id{ internal::invalid_ID }; //link's unique identifier

		Link& operator*() const { return (*links)[index]; }
		Link* operator->() const { return &((*links)[index]); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the link in the links vector
		void Reload();
	private:
		bool valid() const;
	};

	struct AnimRef
	{
		std::vector<Animation>* anims{ nullptr }; //reference to vector of animations
		int index{ -1 };	//animation index
		size_t id{ internal::invalid_ID }; //animation's unique identifier

		Animation& operator*() const { return (*anims)[index]; }
		Animation* operator->() const { return &((*anims)[index]); }

		operator bool() const {
			return valid();
		}

		//recalculates the index by looking for the animation in the animations vector
		void Reload();
	private:
		bool valid() const;
	};

	struct Node
	{
		GroupRef group;
		std::string name{};
		//animation asset loaded from file
		Asset anim_asset{};
		AnimRef anim{};
		////used to get the animation's index
		//size_t animation_ID{ internal::invalid_ID };
		////index of the animation in the animation vector
		//uint animation_index{ internal::invalid_index };
		////Animation animation{};
		float speed{ 1.f };
		glm::vec3 position{};

		size_t node_ID{ internal::invalid_ID };

		//trackers to be given to the animation component 
		//upon reaching this node
		std::vector<ProgressTracker> trackers{};
		//outgoing links to other nodes
		std::vector<LinkRef> outgoingLinks{};

		//Node(Group& _group, std::string const _name = "Unnamed Node");
		Node(NodeInfo& info);
		//void SetAnimation(Asset asset);
		//void SetAnimation(Asset asset);
		Animation& GetAnimation();
	};

	struct NodeInfo
	{
		std::string name{ "Unnamed Node" };
		std::string animation_name{};
		float speed{ 1.f };
		glm::vec3 position{ 0.f,0.f,0.f };

		//dont fill this up
		GroupRef group{};
		size_t nodeID{ internal::invalid_ID };
	};

	

	struct Group
	{
		std::string name{ "Unnamed Group" };
		NodeRef startNode;
		//contains the nodes and their positions to be displayed in the editor
		std::vector<Node> nodes{};
		std::vector<Link> links{};
		AnimationTree* tree{ nullptr };
		size_t groupID{ internal::invalid_ID };	//unique identifier

		//Group(std::string const _name = "Unnamed Group");
		Group(GroupInfo const& info);
	};
	struct GroupInfo
	{
		std::string name{ "Unnamed Group" };
		size_t groupID{ internal::invalid_ID };
		AnimationTree* tree{nullptr};
	};
	

	//variables that are defined within an AnimationTree that can be accessed and assigned values from scripts or editor
	struct Parameter
	{
		using DataType = rttr::variant;

		P_TYPE type{};
		DataType value{};
		size_t paramID{ internal::invalid_ID };
		std::string name{"Unnamed Parameter"};

		Parameter(ParameterInfo const& info);
		void Set(DataType const& _value);
	};

	struct ParameterInfo
	{
		std::string name{ "Unnamed Parameter" };
		P_TYPE type;
		//optional initial value
		Parameter::DataType value;
	};

	struct Condition
	{
		enum class CompareType
		{
			GREATER,
			LESS,
			EQUAL,
			NOT_EQUAL
		};
		using DataType = rttr::variant;
		using CompareFn = bool(DataType const&, DataType const&);
		using CompareFnMap = std::unordered_map< P_TYPE, std::unordered_map<CompareType, CompareFn*>>;
		
		CompareType comparison_type;
		P_TYPE type;
		DataType value{};
		//used to track the parameter's index in the animation tree's vector
		size_t paramID{ internal::invalid_ID };
		uint32_t parameterIndex{};
		CompareFn* compareFn{ nullptr };
		static CompareFnMap comparisonFn_map;


		Condition(ConditionInfo const& info);
		bool Satisfied(AnimationTracker& tracker);
	};

	struct ConditionInfo
	{
		Condition::CompareType comparison{};
		std::string parameter_name{};
		//initial value, leave empty for default
		Condition::DataType value{};
		//dont fill this
		size_t _paramID{ internal::invalid_ID };
		//dont fill this
		Parameter* _param{nullptr};
	};

	struct Link
	{
		NodeRef src;
		NodeRef dst;

		bool has_exit_time{ false };
		float exit_time{ 0.f };
		bool fixed_duration{ false };
		float transition_duration{ 0.f };
		float transition_offset{ 0.f };
		std::string name{"Unnamed Link"};
		std::vector<Condition> conditions{};
		size_t linkID{ internal::invalid_ID };

		Link(NodeRef _src, NodeRef _dst);
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

		std::string timeline_name{ "Unnamed Timeline" };

		std::vector<int> children_index;
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
		static std::unordered_map< std::string, uint> name_to_index;
		static std::unordered_map< size_t, uint> ID_to_index;
		static std::vector<Animation> animation_storage;


		std::string name{ "Unnamed Animation" };

		std::vector<ScriptEvent> events{};

		std::vector<Timeline> timelines{};

		bool looping{ false };
		
		float animation_length{0.f};

		size_t animation_ID{ internal::generateUID() };

		static std::string LoadAnimationFromFBX(std::string const& filepath);
		static Animation* AddAnimation(Animation& anim);

	};	

	

	
}




namespace oo
{
	class AnimationComponent;
}
