/************************************************************************************//*!
\file           AnimationInternal.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
Internal functions used by the animation system implementation.  Not for external use.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"
#include "AnimationKeyFrame.h"
#include "AnimationParameter.h"

#include "Utility/Hash.h"
#include <rapidjson/document.h>
#include <rapidjson/reader.h>

namespace oo::Anim::internal
{
	//serialization
	using SerializeFn = void(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>&, rttr::variant&);
	extern std::unordered_map <StringHash::size_type, rttr::type> hash_to_rttrType;
	extern std::unordered_map <rttr::type::type_id, StringHash::size_type> rttrType_to_hash;
	extern std::unordered_map<rttr::type::type_id, SerializeFn*> serializeDataFn_map;
	extern std::unordered_map<rttr::type::type_id, SerializeFn*> serializeRTTRVariantFn_map;

	//loading
	using LoadFn = rttr::variant(rapidjson::Value&);
	extern std::unordered_map<rttr::type::type_id, LoadFn*> loadDataFn_map;
	extern std::unordered_map<rttr::type::type_id, LoadFn*> loadRTTRVariantFn_map;
	rttr::variant LoadRTTRVariant(rapidjson::Value& value);


	NodeRef CreateNodeReference(Group& group, size_t id);
	NodeRef CreateNodeReference(std::map<size_t, Node>& node_container, size_t id);
	GroupRef CreateGroupReference(AnimationTree& tree, size_t id);
	LinkRef CreateLinkReference(Group& group, size_t id);
	AnimRef CreateAnimationReference(size_t id);
	TimelineRef CreateTimelineReference(AnimRef anim_ref, std::string const& timeline_name);

	Parameter::DataType ParameterDefaultValue(P_TYPE const type);
	Parameter::DataType ConditionDefaultValue(P_TYPE const type);

	bool TypeMatchesDataType(Parameter* parameter, Parameter::DataType const& value);
	bool ConditionSatisfied(Condition& condition, AnimationTracker& tracker);

	void AssignComparisonFunctionToCondition(Condition& condition);
	//checks if currrent time has progressed past target time
	inline bool Withinbounds(float current, float target);
	void TriggerEvent(UpdateProgressTrackerInfo& info, ScriptEvent& event);
	KeyFrame::DataType GetInterpolatedValue(rttr::type rttr_type, KeyFrame::DataType prev, KeyFrame::DataType next, float percentage);



	void UpdateProperty_Animation(UpdateProgressTrackerInfo& info, float updatedTimer);

	void UpdateFBX_Animation(UpdateProgressTrackerInfo& info, float updatedTimer);
	//go through all progress trackers and call their update function
	void UpdateTrackerKeyframeProgress(UpdateTrackerInfo& info, float updatedTimer);

	void ResetTrackerProgress(AnimationTracker& tracker);

	void UpdateScriptEventProgress(UpdateTrackerInfo& info, float updatedTimer);

	KeyFrame* GetCurrentKeyFrame(ProgressTracker& tracker);

	void UpdateTrackerTransitionProgress(UpdateTrackerInfo& info, float updatedTimer);

	void AssignNodeToTracker(AnimationTracker& animTracker, NodeRef node);

	void ResetTriggers(UpdateTrackerInfo& info, Link& link);

	//update a node's trackers to reflect its animation timelines
	void UpdateNodeTrackers(Node& node);
	//checks if a node is available for transition
	Link* CheckNodeTransitions(UpdateTrackerInfo& info, Node& node);
	Link* CheckNodeTransitions(UpdateTrackerInfo& info);

	void ActivateTransition(UpdateTrackerInfo& info, Link* link, Node& current_node);

	void UpdateTracker(UpdateTrackerInfo& info);

	void AssignAnimationTreeToComponent(IAnimationComponent& component, std::string const& name);
	void AssignAnimationTreeToComponent(IAnimationComponent& component, size_t id);

	//RETRIEVAL
	Group* RetrieveGroupFromTree(AnimationTree& tree, std::string const& groupName);
	Node* RetrieveNodeFromTree(AnimationTree& tree, std::string const& groupName, std::string const& name);
	Node* RetrieveNodeFromGroup(Group& group, std::string const& name);
	Node* RetrieveNodeFromGroup(Group& group, UID node_ID);
	//same as RetrieveNodeFromGroup but without error messages and asserts
	Node* TryRetrieveNodeFromGroup(Group& group, std::string const& name);
	Link* RetrieveLinkFromGroup(Group& group, std::string const& linkName);
	Link* RetrieveLinkFromGroup(Group& group, UID link_ID);
	Parameter* RetrieveParameterFromTree(AnimationTree& tree, std::string const& param_name);
	Timeline* RetrieveTimelineFromAnimation(Animation& animation, std::string const& timelineName);
	Timeline* TryRetrieveTimelineFromAnimation(Animation& animation, std::string const& timelineName);
	Parameter* RetrieveParameterFromComponent(IAnimationComponent& comp, std::string const& paramName);
	Parameter* RetrieveParameterFromComponent(IAnimationComponent& comp, size_t id);
	Parameter* RetrieveParameterFromComponentByIndex(IAnimationComponent& comp, uint index);
	Animation* RetrieveAnimation(std::string const& anim_name);
	Animation* RetrieveAnimation(size_t anim_id);
	Animation* RetrieveAnimation(oo::Asset asset);
	AnimationTree* RetrieveAnimationTree(std::string const& name);
	AnimationTree* RetrieveAnimationTree(size_t id);
	
	Node* AddNodeToGroup(Group& group, Anim::NodeInfo& info);

	Group* AddGroupToTree(AnimationTree& tree, GroupInfo& info);

	Timeline* AddTimelineToAnimation(Animation& animation, Anim::TimelineInfo& info);


	bool KeyframeMatchesTimeline(Timeline& timeline, KeyFrame const& keyframe);

	KeyFrame* AddKeyframeToTimeline(Timeline& timeline, KeyFrame const& keyframe);

	ScriptEvent* AddScriptEventToAnimation(Animation& animation, ScriptEvent const& scriptevent);

	Link* AddLinkBetweenNodes(Group& group, std::string const& src_name, std::string const& dst_name);

	Parameter* AddParameterToTree(AnimationTree& tree, Anim::ParameterInfo const& info);
	void RemoveParameterFromTree(AnimationTree& tree, UID param_ID);

	Condition* AddConditionToLink(AnimationTree& tree, Link& link, ConditionInfo& info);
	void RemoveConditionFromLink(Link& link, UID conditionID);

	Animation* AddAnimationToNode(Node& node, Animation& anim);
	Animation* AddAnimationToNode(Node& node, oo::Asset asset);

	bool RemoveNodeFromGroup(Group& group, UID node_ID);
	void RemoveLinkFromGroup(Group& group, UID link_ID);


	void LoadFBX(std::string const& filepath, Animation* anim);



	void InitialiseComponent(IAnimationComponent& comp);

	void BindConditionToParameter(AnimationTree& tree, Condition& condition);

	void BindConditionsToParameters(AnimationTree& tree);

	void BindNodesToAnimations(AnimationTree& tree);


	void CalculateAnimationLength(AnimationTree& tree);
	void CalculateAnimationLength(Animation& anim);

	void ReloadReferences(AnimationTree& tree);

	void ReAssignReferences(AnimationTree& tree);

	void CalculateParameterIndexes(AnimationTree& tree);

	Animation* AddAnimationToStorage(std::string const& name);

	//parameters
	uint GetParameterIndex(IAnimationComponent& comp, std::string const& paramName);
	
	//timeline
	void ReInsertKeyFrame(Timeline& timeline, uint index, float time);
}