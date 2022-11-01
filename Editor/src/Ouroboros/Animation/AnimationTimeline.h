/************************************************************************************//*!
\file           AnimationTimeline.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A collection of transitions between two nodes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"

namespace oo::Anim
{
	//stores a collection of Keyframes that edit a specific value in a component
	struct Timeline
	{
		enum class TYPE : int
		{
			PROPERTY,
			FBX_ANIM,
			SCRIPT_EVENT
		};

		enum class DATATYPE : int
		{
			VEC3,
			QUAT,
			BOOL,
		};
		std::string name{ "Unnamed Timeline" };
		//what kind of timeline is it for, script events, or property, or fbx animation
		const TYPE type{};
		const DATATYPE datatype{};
		std::vector<int> children_index;
		std::vector<KeyFrame> keyframes;
		//function pointer to get the component
		Ecs::GetCompFn* get_componentFn{ nullptr };
		rttr::type rttr_type;
		rttr::property rttr_property;
		size_t component_hash;

		//Timeline(TYPE _type, DATATYPE _datatype, std::string const _name = "Unnamed Timeline");
		Timeline(TimelineInfo const& info);
		RTTR_ENABLE();
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

		bool hierarchy_provided{ false };
	};

	//tracks progress in 1 timeline
	struct ProgressTracker
	{
		using UpdateFn = void(*)(internal::UpdateProgressTrackerInfo&, float);

		Timeline::TYPE type{};
		size_t index{ 0ul };	//last index of whatever keyframe we were at
		UpdateFn updatefunction{ nullptr };
		Timeline* timeline{ nullptr };

		ProgressTracker(const Timeline::TYPE _type);
		static ProgressTracker Create(Timeline::TYPE type);
	};
}