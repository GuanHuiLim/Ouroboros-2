/************************************************************************************//*!
\file           AnimationComponent.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          This component allows a gameobject to have animations

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"
#include "AnimationComponent.h"
#include "AnimationInternal.h"

#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<AnimationComponent>("AnimationComponent")
            //.property("AnimationTreeName", &AnimationComponent::GetAnimationTreeName, static_cast<void (AnimationComponent::*)(std::string)>(&AnimationComponent::SetAnimationTree))
            .property("Animation Tree", &AnimationComponent::GetAnimationTreeAsset, static_cast<void (AnimationComponent::*)(oo::Asset)>(&AnimationComponent::SetAnimationTree))
			(
				metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::AnimationTree))
			)
		;
    }


	Anim::IAnimationComponent& AnimationComponent::GetActualComponent()
	{
		return actualComponent;
	}

	void AnimationComponent::Set_Root_Entity(Ecs::EntityID entity)
	{
		actualComponent.root_objectID = root_object = entity;
	}

	void AnimationComponent::SetAnimationTree(std::string name)
	{
		actualComponent.Reset();
		oo::Anim::internal::AssignAnimationTreeToComponent(actualComponent, name);
	}

	void oo::AnimationComponent::SetAnimationTree(size_t id)
	{
		actualComponent.Reset();
		oo::Anim::internal::AssignAnimationTreeToComponent(actualComponent, id);
	}

	void oo::AnimationComponent::SetAnimationTree(oo::Asset asset)
	{
		if (asset.IsValid() == false) return;

		auto treeID = asset.GetData<Anim::UID>();
		SetAnimationTree(treeID);
		//if failed to find animation tree
		if (HasAnimationTree() == false) return;
		//otherwise set it as asset
		anim_tree_asset = asset;
	}

	std::string AnimationComponent::GetAnimationTreeName()
	{
		if (HasAnimationTree() == false) return {};

		return actualComponent.animTree->name;
	}

	bool oo::AnimationComponent::HasAnimationTree()
	{
		return actualComponent.animTree != nullptr;
	}

	void AnimationComponent::SetParameter(std::string const& name, Anim::Parameter::DataType value)
	{
		auto parameter = oo::Anim::internal::RetrieveParameterFromComponent(actualComponent, name);
		assert(parameter);
		assert(value.get_type() == parameter->value.get_type());
		parameter->value = value;
	}


	void AnimationComponent::SetParameterByID(size_t id, Anim::Parameter::DataType value)
	{
		auto parameter = oo::Anim::internal::RetrieveParameterFromComponent(actualComponent, id);
		assert(parameter);
		assert(value.get_type() == parameter->value.get_type());
		parameter->value = value;
	}


	void AnimationComponent::SetParameterByIndex(uint index, Anim::Parameter::DataType value)
	{
		auto parameter = oo::Anim::internal::RetrieveParameterFromComponentByIndex(actualComponent, index);
		assert(parameter);
		assert(value.get_type() == parameter->value.get_type());
		parameter->value = value;
	}

	size_t AnimationComponent::GetParameterID(std::string const& name)
	{
		auto parameter = oo::Anim::internal::RetrieveParameterFromComponent(actualComponent, name);
		assert(parameter);
		return parameter->paramID;
	}

	uint AnimationComponent::GetParameterIndex(std::string const& name)
	{
		return oo::Anim::internal::GetParameterIndex(actualComponent, name);
	}

	Anim::AnimationTree* AnimationComponent::GetAnimationTree()
	{
		return actualComponent.animTree;
	}
	oo::Asset oo::AnimationComponent::GetAnimationTreeAsset()
	{
		return anim_tree_asset;
	}
	Anim::AnimationTracker& AnimationComponent::GetTracker()
	{
		return actualComponent.tracker;
	}

	Anim::GroupRef AnimationComponent::GetGroup(std::string const& name)
	{
		auto tree = GetAnimationTree();
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return {};
		}

		auto group = oo::Anim::internal::RetrieveGroupFromTree(*tree, name);

		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Cannot find {0} group!!", name, GetAnimationTree()->name);
			assert(false);
			return {};
		}

		return oo::Anim::internal::CreateGroupReference(*tree, group->groupID);
	}



	Anim::NodeRef AnimationComponent::AddNode(std::string const& groupName, Anim::NodeInfo& info)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return {};
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, groupName);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add node!!", groupName);
			assert(false);
			return {};
		}
		info.group = Anim::internal::CreateGroupReference(*tree, group->groupID);
		auto node = Anim::internal::AddNodeToGroup(*group, info);
		//node should exist after adding to group
		if (node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Adding {0} node to {1} animation tree failed!!", info.name, GetAnimationTree()->name);
			assert(false);
			return {};
		}
		return oo::Anim::internal::CreateNodeReference(*group,node->node_ID);
	}

	void oo::AnimationComponent::RemoveNode(Anim::TargetNodeInfo const& info)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return;
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, info.group_name);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot remove node!!", info.group_name);
			assert(false);
			return;
		}
		Anim::internal::RemoveNodeFromGroup(*group, info.node_ID);
	}

	Anim::LinkRef AnimationComponent::AddLink(std::string const& groupName, std::string const& src, std::string const& dst)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return {};
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, groupName);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add link!!", groupName);
			assert(false);
			return {};
		}

		auto src_node = Anim::internal::RetrieveNodeFromGroup(*group, src);
		auto dst_node = Anim::internal::RetrieveNodeFromGroup(*group, dst);
		//src and dst nodes should exist
		if (src_node == nullptr || dst_node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Link from {0} node to {1} cannot be added!!", src, dst);
			LOG_CORE_DEBUG_INFO("{0} node does not exist!!", ((src_node) ? dst : src));
			assert(false);
			return {};
		}

		auto link = Anim::internal::AddLinkBetweenNodes(*group, src, dst);

		if (link == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Link from {0} node to {1} cannot be added!!", src, dst);
			assert(false);
			return {};
		}

		return oo::Anim::internal::CreateLinkReference(*group,link->linkID);
	}

	void oo::AnimationComponent::RemoveLink(Anim::TargetLinkInfo const& info)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return;
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, info.group_name);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot remove link!!", info.group_name);
			assert(false);
			return;
		}
		Anim::internal::RemoveLinkFromGroup(*group, info.link_ID);
	}

	Anim::Parameter* AnimationComponent::AddParameter(Anim::ParameterInfo const& info)
	{
		if (GetAnimationTree() == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return nullptr;
		}

		auto parameter = oo::Anim::internal::AddParameterToTree(*GetAnimationTree(), info);

		if (parameter == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Parameter {0} cannot be added!!", info.name);
			assert(false);
		}

		return parameter;
	}

	Anim::Condition* AnimationComponent::AddCondition(std::string const& groupName, std::string const& linkName, Anim::ConditionInfo info)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return nullptr;
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, groupName);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add condition!!", groupName);
			assert(false);
			return nullptr;
		}
		auto link = Anim::internal::RetrieveLinkFromGroup(*group, linkName);
		//link should exist
		if (link == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} link not found, cannot add condition!!", linkName);
			assert(false);
			return nullptr;
		}
		auto parameter = Anim::internal::RetrieveParameterFromTree(*tree, info.parameter_name);
		//parameter should exist
		if (parameter == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} parameter not found, cannot add condition!!", info.parameter_name);
			assert(false);
			return nullptr;
		}

		info._param = parameter;
		auto condition = Anim::internal::AddConditionToLink(*tree, *link, info);

		if (condition == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Condition cannot be added to Link {0}!!", linkName);
			assert(false);
		}

		Anim::internal::BindConditionToParameter(*tree, *condition);

		return condition;
	}

	Anim::TimelineRef AnimationComponent::AddTimeline(std::string const& groupName, std::string const& nodeName,
		Anim::TimelineInfo& info)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return {};
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, groupName);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add timeline!!", groupName);
			assert(false);
			return {};
		}
		auto node = Anim::internal::RetrieveNodeFromGroup(*group, nodeName);
		//node should exist
		if (node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} node not found, cannot add timeline!!", nodeName);
			assert(false);
			return {};
		}

		//auto timeline = Anim::internal::AddTimelineToAnimation(node->GetAnimation(), timelineName, type, datatype);
		auto timeline = Anim::internal::AddTimelineToAnimation(node->GetAnimation(), info);

		if (timeline == nullptr)
		{
			LOG_CORE_DEBUG_INFO("failed to add {0} timeline!!", info.timeline_name);
			assert(false);
			return {};
		}
		//update the node's trackers to reflect the new timeline
		Anim::internal::UpdateNodeTrackers(*node);

		return Anim::internal::CreateTimelineReference(Anim::internal::CreateAnimationReference(node->GetAnimation().animation_ID), timeline->name);
	}

	//Anim::Timeline* AnimationComponent::AddTimeline(std::string const& groupName, std::string const& nodeName,
	//	std::string const& timelineName, Anim::Timeline::TYPE type,
	//	Anim::Timeline::DATATYPE datatype)
	//{
	//	auto tree = GetAnimationTree();
	//	//tree should exist
	//	if (tree == nullptr)
	//	{
	//		LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
	//		return nullptr;
	//	}
	//	auto group = Anim::internal::RetrieveGroupFromTree(*tree, groupName);
	//	//group should exist
	//	if (group == nullptr)
	//	{
	//		LOG_CORE_DEBUG_INFO("{0} group not found, cannot add timeline!!", groupName);
	//		return nullptr;
	//	}
	//	auto node = Anim::internal::RetrieveNodeFromGroup(*group, nodeName);
	//	//node should exist
	//	if (node == nullptr)
	//	{
	//		LOG_CORE_DEBUG_INFO("{0} node not found, cannot add timeline!!", nodeName);
	//		return nullptr;
	//	}

	//	auto timeline = Anim::internal::AddTimelineToAnimation(node->GetAnimation(), timelineName, type, datatype);
	//	if (timeline == nullptr)
	//	{
	//		LOG_CORE_DEBUG_INFO("failed to add {0} timeline!!", timelineName);
	//		return nullptr;
	//	}

	//	return timeline;
	//}

	Anim::KeyFrame* AnimationComponent::AddKeyFrame(std::string const& groupName, std::string const nodeName,
		std::string const& timelineName, Anim::KeyFrame keyframe)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return nullptr;
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, groupName);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add keyframe!!", groupName);
			assert(false);
			return nullptr;
		}
		auto node = Anim::internal::RetrieveNodeFromGroup(*group, nodeName);
		//node should exist
		if (node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} node not found, cannot add keyframe!!", nodeName);
			assert(false);
			return nullptr;
		}
		auto timeline = Anim::internal::RetrieveTimelineFromAnimation(node->GetAnimation(), timelineName);
		//timeline should exist
		if (timeline == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} timeline not found, cannot add keyframe!!", timelineName);
			assert(false);
			return nullptr;
		}

		auto created_keyframe = Anim::internal::AddKeyframeToTimeline(*timeline, keyframe);
		if (created_keyframe == nullptr)
		{
			LOG_CORE_DEBUG_INFO("couldn't add keyframe!!");
			assert(false);
			return nullptr;
		}

		return created_keyframe;
	}

	Anim::ScriptEvent* AnimationComponent::AddScriptEvent(std::string const& groupName, std::string const nodeName,
		std::string const& timelineName, Anim::ScriptEvent scriptevent)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return nullptr;
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, groupName);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add script event!!", groupName);
			assert(false);
			return nullptr;
		}
		auto node = Anim::internal::RetrieveNodeFromGroup(*group, nodeName);
		//node should exist
		if (node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} node not found, cannot add script event!!", nodeName);
			assert(false);
			return nullptr;
		}

		auto created_scriptevent = Anim::internal::AddScriptEventToAnimation(node->GetAnimation(), scriptevent);
		if (created_scriptevent == nullptr)
		{
			LOG_CORE_DEBUG_INFO("couldn't add keyframe!!");
			assert(false);
			return nullptr;
		}

		return created_scriptevent;
	}

	Anim::AnimRef AnimationComponent::SetNodeAnimation(Anim::SetNodeAnimInfo const& info)
	{
		auto tree = GetAnimationTree();
		//tree should exist
		if (tree == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return {};
		}
		auto group = Anim::internal::RetrieveGroupFromTree(*tree, info.group_name);
		//group should exist
		if (group == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add animation to node!!", info.group_name);
			assert(false);
			return {};
		}
		auto node = Anim::internal::RetrieveNodeFromGroup(*group, info.node_ID);
		//node should exist
		if (node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} node not found, cannot add animation to node!!", info.node_ID);
			assert(false);
			return {};
		}

		auto anim = Anim::internal::RetrieveAnimation(info.anim_asset);
		//animation should exist
		if (anim == nullptr)
		{
			LOG_CORE_DEBUG_INFO("animation not found, cannot add animation to node!!");
			assert(false);
			return {};
		}

		auto result = Anim::internal::AddAnimationToNode(*node, *anim);
		if (result == nullptr)
		{
			LOG_CORE_DEBUG_INFO("error, cannot add animation to node!!");
			assert(false);
			return {};
		}
		return oo::Anim::internal::CreateAnimationReference(result->animation_ID);
	}
	void oo::AnimationComponent::ReInsertKeyFrame(Anim::TargetTimelineInfo info, uint keyframe_index, float new_time)
	{
		auto anim = Anim::internal::RetrieveAnimation(info.anim_id);
		//animation should exist
		if (anim == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} animation not found, cannot recalculate KeyFrames!!", info.anim_id);
			assert(false);
			return;
		}
		auto timeline = oo::Anim::internal::RetrieveTimelineFromAnimation(*anim, info.timeline_name);
		//timeline should exist
		if (timeline == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} timeline not found, cannot recalculate KeyFrames!!", info.timeline_name);
			assert(false);
			return;
		}

		oo::Anim::internal::ReInsertKeyFrame(*timeline, keyframe_index, new_time);
	}
}

