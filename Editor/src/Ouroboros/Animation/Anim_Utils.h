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
	extern std::unordered_map< size_t, rttr::instance(*)(void*)> hash_to_instance;

	void Initialise_hash_to_instance();
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
	struct KeyFrame;
	struct ScriptEvent;
	struct TimelineInfo;
	struct Timeline;
	struct Animation;		//represents property or fbx file animations
	struct Node;			//a node in the animation tree
	struct Group;
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

	//variables that are defined within an AnimationTree that can be accessed and assigned values from scripts or editor
	struct Parameter
	{
		using DataType = rttr::variant;

		P_TYPE type{};
		DataType value{};
		size_t paramID{};
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
		size_t paramID{};
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
		size_t _paramID{};
		//dont fill this
		Parameter* _param{nullptr};
	};

	struct Link
	{
		Node& src;
		Node& dst;

		bool has_exit_time{ false };
		float exit_time{ 0.f };
		bool fixed_duration{ false };
		std::string name{"Unnamed Link"};

		std::vector<Condition> conditions{};

		Link(Node& _src, Node& _dst);
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
		std::string name{ "Unnamed Animation" };

		std::vector<ScriptEvent> events{};

		std::vector<Timeline> timelines{};

		bool looping{ false };
		//std::vector<ProgressTracker> trackers{};
	};

	struct Node
	{
		Group& group;
		std::string name{};
		//animation asset loaded from file
		Animation animation{};
		float speed{1.f};
		glm::vec3 position{};
		
		//trackers to be given to the animation component 
		//upon reaching this node
		std::vector<ProgressTracker> trackers{};	
		//outgoing links to other nodes
		std::vector<Link*> outgoingLinks{};

		Node(Group& _group, std::string const _name = "Unnamed Node");
	};

	struct NodeRef
	{
		std::vector<Node>& nodes;
		int index{ -1 };

		Node& operator*() const { return nodes[index]; }
		Node* operator->() const { return &(nodes[index]); }

		operator bool() const { return index >= 0 && index < nodes.size(); }
	};

	struct Group
	{
		std::string name{ "Unnamed Group"};
		NodeRef startNode;
		//contains the nodes and their positions to be displayed in the editor
		std::vector<Node> nodes{};
		std::vector<Link> links{};
		AnimationTree* tree{nullptr};

		Group(std::string const _name = "Unnamed Group");
	};
	
}




namespace oo
{
	class AnimationComponent;
}
