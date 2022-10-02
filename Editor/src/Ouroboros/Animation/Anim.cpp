/************************************************************************************//*!
\file           Anim.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          Definitions for animation related classes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "Anim.h"
#include "AnimationComponent.h"
#include "AnimationSystem.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "Ouroboros/Vulkan/RendererComponent.h"
#include "Ouroboros/Core/Input.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "Project.h"
#include "App/Editor/UI/Tools/MeshHierarchy.h"

#include <random>
namespace oo::Anim::internal
{
	uint GetAnimationIndex(std::string const& name)
	{
		assert(Animation::name_to_index.contains(name));
		return Animation::name_to_index[name];
	}
	uint GetAnimationIndex(size_t id)
	{
		assert(Animation::ID_to_index.contains(id));
		return Animation::ID_to_index[id];
	}
	NodeRef CreateNodeReference(Group& group, size_t id)
	{
		int index = 0;
		for (auto& node : group.nodes)
		{
			if (node.node_ID == id)
			{
				NodeRef ref{
					.nodes{&group.nodes},
					.index{index},
					.id{id}
				};
				return ref;
			}
			++index;
		}
		assert(false);
		return {};
	}
	NodeRef CreateNodeReference(std::vector<Node>& node_container, size_t id)
	{
		int index = 0;
		for (auto& node : node_container)
		{
			if (node.node_ID == id)
			{
				NodeRef ref{
					.nodes{&node_container},
					.index{index},
					.id{id} 
				};

				return ref;
			}
			++index;
		}
		assert(false);
		return {};
	}
	GroupRef CreateGroupReference(AnimationTree& tree, size_t id)
	{
		int index = 0;
		for (auto& group : tree.groups)
		{
			if (group.groupID == id)
			{
				GroupRef ref{
					.groups{&tree.groups},
					.index{index},
					.id{id}
				};

				return ref;
			}
			++index;
		}
		assert(false);
		return {};
	}
	LinkRef CreateLinkReference(Group& group, size_t id)
	{
		int index = 0;
		for (auto& link : group.links)
		{
			if (link.linkID == id)
			{
				LinkRef ref{
					.links{&group.links},
					.index{index},
					.id{id}
				};

				return ref;
			}
			++index;
		}

		assert(false);
		return {};
	}
	

	Parameter::DataType ParameterDefaultValue(P_TYPE const type)
	{
		switch (type)
		{
			case P_TYPE::BOOL:
			case P_TYPE::TRIGGER:
				return false;
				break;
			case P_TYPE::INT:
				return 0;
				break;
			case P_TYPE::FLOAT:
				return 0.f;
				break;
		}

		assert(false);//unknown type?!?!
		return false;
	}

	Parameter::DataType ConditionDefaultValue(P_TYPE const type)
	{
		switch (type)
		{
		case P_TYPE::BOOL:
		case P_TYPE::TRIGGER:
			return true;
			break;
		case P_TYPE::INT:
			return 0;
			break;
		case P_TYPE::FLOAT:
			return 0.f;
			break;
		}

		assert(false); //unknown type?!?!
		return true;
	}


	bool TypeMatchesDataType(Parameter* parameter, Parameter::DataType const& value)
	{
		if (value.is_type<bool>())
			return parameter->type == P_TYPE::BOOL || parameter->type == P_TYPE::TRIGGER;
	
		if (value.is_type<int>())
			return parameter->type == P_TYPE::INT;

		if (value.is_type<float>())
			return parameter->type == P_TYPE::FLOAT;

		assert(false);
		return false;
	}

	bool ConditionSatisfied(Condition& condition, AnimationTracker& tracker)
	{
		//bool result = (condition->parameter->value == condition->value);

		////if its a trigger, comsume the trigger and set it back to false
		//if (condition->type == P_TYPE::TRIGGER && result)
		//	condition->parameter->SetWithoutChecking(false);
		assert(condition.compareFn);
		if (condition.compareFn)
			return condition.compareFn(condition.value, tracker.parameters[condition.parameterIndex].value);

		return false;
	}

	void AssignComparisonFunctionToCondition(Condition& condition)
	{
		assert(Condition::comparisonFn_map.contains(condition.type));

		if (Condition::comparisonFn_map[condition.type].contains(condition.comparison_type))
			condition.compareFn = Condition::comparisonFn_map[condition.type][condition.comparison_type];
		else
			condition.compareFn = nullptr;
	}

	//checks if currrent time has progressed past target time
	inline bool Withinbounds(float current, float target)
	{
		return target < current;
	}

	void TriggerEvent(ScriptEvent& event)
	{
		//TODO
		assert(false);
	}

	KeyFrame::DataType GetInterpolatedValue(rttr::type rttr_type, KeyFrame::DataType prev, KeyFrame::DataType next, float percentage)
	{
		if (rttr_type == rttr::type::get< glm::vec3>())
		{
			return (1.f - percentage) * prev.get_value< glm::vec3 >() + (percentage * next.get_value< glm::vec3 >());
		}
		else if (rttr_type == rttr::type::get< glm::quat>())
		{
			return (1.f - percentage) * prev.get_value< glm::quat >() + (percentage * next.get_value< glm::quat >());
		}
		else if (rttr_type == rttr::type::get< bool>())
		{
			return percentage > 0.5f ? next : prev;
		}
		else return prev;
	}

	//void UpdateEvent(AnimationComponent& comp, AnimationTracker& tracker, ProgressTracker& progressTracker, float updatedTimer)
	void UpdateEvent(UpdateProgressTrackerInfo& info, float updatedTimer)
	{
		//it should be an event tracker
		assert(info.progressTracker.type == Timeline::TYPE::SCRIPT_EVENT);

		auto& timeline = info.tracker_info.tracker.currentNode->GetAnimation().events;
		//no events in timeline
		if (timeline.empty()) return;
		//already hit last so we return
		if (info.progressTracker.index >= (timeline.size() - 1ul))
			return;

		auto& nextEvent = *(timeline.begin() + info.progressTracker.index + 1ul);

		//if next event not within bounds
		if (Withinbounds(updatedTimer, nextEvent.time) == false) return;
			
		TriggerEvent(nextEvent);
		info.progressTracker.index++;

	}

	//void UpdateProperty_Animation(AnimationComponent& comp, AnimationTracker& tracker, ProgressTracker& progressTracker, float updatedTimer)
	void UpdateProperty_Animation(UpdateProgressTrackerInfo& info, float updatedTimer)
	{
		//verify progress tracker correct type
		assert(info.progressTracker.type == Timeline::TYPE::PROPERTY);
		//it should be linked to a timeline!!
		assert(info.progressTracker.timeline != nullptr);


		auto& timeline = *(info.progressTracker.timeline);
		//verify correct timeline type
		assert(timeline.type == Timeline::TYPE::PROPERTY);
		//no keyframes so we return
		if (timeline.keyframes.empty()) return;

		//already hit last and animation not looping so we return
		if (info.progressTracker.index >= (timeline.keyframes.size() - 1ul) &&
			info.tracker_info.tracker.currentNode->GetAnimation().looping == false)
		{
			return;
		}

		////if looping, set the normalized time based on iterations
		//if (info.tracker_info.tracker.currentNode->GetAnimation().looping)
		//{
		//	if (info.tracker_info.tracker.timer > timeline.keyframes.back().time)
		//	{
		//		currentTimer -= timeline.keyframes.back().time;
		//		num_iterations += 1.f;
		//	}
		//	//set normalized timer
		//	info.tracker_info.tracker.normalized_timer = num_iterations + (currentTimer / timeline.keyframes.back().time);
		//}
		//else
		//{
		//	//set normalized timer
		//	info.tracker_info.tracker.normalized_timer = (updatedTimer / timeline.keyframes.back().time);
		//}

		//if next keyframe within bounds increment index 
		auto& nextEvent = *(timeline.keyframes.begin() + info.progressTracker.index + 1ul);
		if (Withinbounds(updatedTimer, nextEvent.time))
		{
			info.progressTracker.index++;

			//went past last keyframe so we set data to last and return
			if (info.progressTracker.index >= (timeline.keyframes.size() - 1ul))
			{
				//get the instance
				auto ptr = info.tracker_info.system.Get_Ecs_World()->get_component(
					info.tracker_info.entity, info.progressTracker.timeline->component_hash);
				auto rttr_instance = hash_to_instance[info.progressTracker.timeline->component_hash](ptr);
				//set the value
				info.progressTracker.timeline->rttr_property.set_value(rttr_instance, timeline.keyframes.back().data);

				//if animation is looping, reset keyframe index
				if (info.tracker_info.tracker.currentNode->GetAnimation().looping)
				{
					info.progressTracker.index = 0;
				}
				return;
			}
			
		}

		/*--------------------------------
		interpolate animation accordingly
		--------------------------------*/
		auto& prevKeyframe = *(timeline.keyframes.begin() + info.progressTracker.index);
		auto& nextKeyframe = *(timeline.keyframes.begin() + info.progressTracker.index + 1u);
		auto prevTime = prevKeyframe.time;
		auto nextTime = nextKeyframe.time;

		//current progress in keyframe over total time in between keyframes
		float percentage = (updatedTimer - prevTime) / (nextTime - prevTime);

		KeyFrame::DataType interpolated_value = GetInterpolatedValue(
			info.progressTracker.timeline->rttr_type, prevKeyframe.data, nextKeyframe.data, percentage);
		
		/*--------------------------------
		set related game object's data
		--------------------------------*/
		GameObject go_root{ info.tracker_info.entity,info.tracker_info.system.Get_Scene() };
		GameObject go{ go_root };
		//traverse the hierarchy
		for (auto& index : timeline.children_index)
		{
			auto children = go.GetDirectChilds();
			go = children[index];
		}
		//get a ptr to the component
		auto ptr = info.tracker_info.system.Get_Ecs_World()->get_component(
			go.GetEntity(), info.progressTracker.timeline->component_hash);

		//get the instance
		auto rttr_instance = hash_to_instance[info.progressTracker.timeline->component_hash](ptr);
		//set the value
		info.progressTracker.timeline->rttr_property.set_value(rttr_instance, interpolated_value);

		//SetComponentData(timeline, interpolated_value);
		//assert(false);
	}

	//void UpdateFBX_Animation(AnimationComponent& comp, AnimationTracker& tracker, ProgressTracker& progressTracker, float updatedTimer)
	void UpdateFBX_Animation(UpdateProgressTrackerInfo& info, float updatedTimer)
	{
		//assert(progressTracker.type == Timeline::TYPE::FBX_ANIM);
	}
	//go through all progress trackers and call their update function
	void UpdateTrackerKeyframeProgress(UpdateTrackerInfo& info, float updatedTimer)
	{
		for (auto& progressTracker : info.tracker.trackers)
		{
			//it should have an update function!!
			assert(progressTracker.updatefunction != nullptr);
			UpdateProgressTrackerInfo p_info{ info, progressTracker };
			//call the respective update function on this tracker
			progressTracker.updatefunction(p_info, updatedTimer);
		}
	}

	KeyFrame* GetCurrentKeyFrame(ProgressTracker& tracker)
	{
		return &(tracker.timeline->keyframes[tracker.index]);
	}

	void UpdateTrackerTransitionProgress(UpdateTrackerInfo& info, float updatedTimer)
	{
		auto& trans_info = info.tracker.transition_info;
		trans_info.transition_timer += info.dt;
		//if timer still not past offset, continue as normal
		if (trans_info.transition_timer < trans_info.transition_offset)
		{
			UpdateTrackerKeyframeProgress(info, updatedTimer);
			return;
		}
		//interpolate between the src and dst keyframes
		auto src_percentage = (trans_info.transition_timer - trans_info.transition_offset) / trans_info.transition_duration;
		auto dst_percentage = 1.f - src_percentage;
		(void)dst_percentage;
		//TODO: there is multiple keyframes, and we can only interpolate the same type ones
		/*for (auto& trackers : info.tracker.trackers)
		{
			
		}*/
		
		//auto kf = GetCurrentKeyFrame(info.tracker.trackers)

	}
	//void UpdateTrackerKeyframeProgress(AnimationComponent& component, AnimationTracker& tracker, float updatedTimer)
	//{
	//	for (auto& progressTracker : tracker.trackers)
	//	{
	//		//it should have an update function!!
	//		assert(progressTracker.updatefunction != nullptr);
	//		//call the respective update function on this tracker
	//		progressTracker.updatefunction(component, tracker, progressTracker, updatedTimer);
	//	}
	//}
	
	//set current node
	//copy the node's trackers
	//reset timer to 0.0f
	//void SetTrackerCurrentNode(AnimationTracker& tracker, Node& node)
	//{
	//	tracker.currentNode = &node;
	//	//tracker.trackers = node.trackers;
	//	tracker.timer = 0.0f;
	//}

	void AssignNodeToTracker(AnimationTracker& animTracker, NodeRef node)
	{
		//set current node
		animTracker.currentNode = node;
		//reset timers
		animTracker.timer = 0.f;
		animTracker.normalized_timer = 0.f;
		animTracker.global_timer = 0.f;
		animTracker.num_iterations = 0;
		//track all timelines in node's animations with trackers
		animTracker.trackers = node->trackers;
	}

	//copy animation tree's parameters to the tracker
	//set the starting node for the tracker and its respective data
	/*void InitializeTracker(IAnimationComponent& comp)
	{
		comp.tracker.parameters = comp.animTree->parameters;
		SetTrackerCurrentNode(comp.tracker, *(comp.animTree->groups.front().startNode));

	}*/

	//update a node's trackers to reflect its animation timelines
	void UpdateNodeTrackers(Node& node)
	{
		node.trackers.clear();
		//TODO: script event tracker

		//add timeline trackers
		for (auto& timeline : node.GetAnimation().timelines)
		{
			auto progressTracker = ProgressTracker::Create(timeline.type);
			//assign it the timeline
			progressTracker.timeline = &timeline;		
			//assign it to the node
			node.trackers.emplace_back(std::move(progressTracker));
		}
	}
	//checks if a node is available for transition
	Link* CheckNodeTransitions(UpdateTrackerInfo& info)
	{
		for (auto& link : info.tracker.currentNode->outgoingLinks)
		{
			if (link->has_exit_time)
			{
				//if exit time not reached continue
				if (info.tracker.normalized_timer < link->exit_time)
					continue;
				//if no conditions return link
				if (link->conditions.empty())
					return &(*link);

				//Check conditions
				bool all_cleared = true;
				for (auto& condition : link->conditions)
				{
					if (condition.Satisfied(info.tracker) == false)
						all_cleared = false;
				}
				if (all_cleared)
					return &(*link);

				//continue if not all conditions met
				continue;
			}
			else //no exit time so just check conditions
			{
				//Check conditions
				bool all_cleared = true;
				for (auto& condition : link->conditions)
				{
					if (condition.Satisfied(info.tracker) == false)
						all_cleared = false;
				}
				if (all_cleared)
					return &(*link);

				continue;
			}			
		}

		return nullptr;
	}

	void ActivateTransition(UpdateTrackerInfo& info, Link* link)
	{
		AssignNodeToTracker(info.tracker, link->dst);
		
		//TODO: transitions
		/*info.tracker.transition_info.in_transition = true;
		info.tracker.transition_info.link = link;
		info.tracker.transition_info.transition_timer = 0.f;
		info.tracker.transition_info.transition_timer = link->transition_offset;
		info.tracker.transition_info.transition_duration = link->transition_duration;

		info.tracker.transition_info.trackers = link->dst->trackers;*/

	}

	void UpdateTracker(UpdateTrackerInfo& info)
	{
		auto result = CheckNodeTransitions(info);
		if (result)
		{
			ActivateTransition(info, result);
		}
		float updatedTimer = info.tracker.timer + info.tracker.currentNode->speed * info.dt;
		//update tracker timer and global timer
		info.tracker.timer = updatedTimer;
		info.tracker.global_timer += info.tracker.currentNode->speed * info.dt;
		info.tracker.normalized_timer = updatedTimer / info.tracker.currentNode->GetAnimation().animation_length;
		
		//not in transition
		if (info.tracker.transition_info.in_transition == false)
		{
			//check if we passed a keyframe and update
			UpdateTrackerKeyframeProgress(info, updatedTimer);
		}
		else
		{
			//interpolate between src and dst nodes
			UpdateTrackerTransitionProgress(info, updatedTimer);
		}
		//update timer and iterations if animation is looping
		if (info.tracker.currentNode->GetAnimation().looping &&
			updatedTimer > info.tracker.currentNode->GetAnimation().animation_length)
		{
			info.tracker.timer = updatedTimer - info.tracker.currentNode->GetAnimation().animation_length;
			++info.tracker.num_iterations;
		}
		
	}

	void AssignAnimationTreeToComponent(IAnimationComponent& component, std::string const& name)
	{
		//retrieve tree
		if (AnimationTree::map.contains(name) == false)
		{
			LOG_CORE_DEBUG_CRITICAL("cannot find animation tree {0}!!", name);
			assert(false);
			return;
		}

		component.animTree = &(AnimationTree::map[name.c_str()]);
	}

	Group* RetrieveGroupFromTree(AnimationTree& tree, std::string const& groupName)
	{
		for (auto& group : tree.groups)
		{
			if (group.name == groupName)
				return &group;
		}

		LOG_CORE_ERROR("could not find {0} group!!", groupName);
		assert(false);
		return nullptr;
	}

	Node* RetrieveNodeFromTree(AnimationTree& tree, std::string const& groupName, std::string const& name)
	{
		auto group = RetrieveGroupFromTree(tree, groupName);

		if (group == nullptr) {
			assert(false);
			return nullptr;
		}

		for (auto& node : group->nodes)
		{
			if (node.name == name)
				return &node;
		}

		LOG_CORE_ERROR("could not find {0} node!!", groupName);
		assert(false);
		return nullptr;
	}

	Node* RetrieveNodeFromGroup(Group& group, std::string const& name)
	{
		for (auto& node : group.nodes)
		{
			if (node.name == name)
				return &node;
		}

		LOG_CORE_ERROR("could not retrieve {0} node in {1} group!!", name, group.name);
		assert(false);
		return nullptr;
	}
	//same as RetrieveNodeFromGroup but without error messages and asserts
	Node* TryRetrieveNodeFromGroup(Group& group, std::string const& name)
	{
		for (auto& node : group.nodes)
		{
			if (node.name == name)
				return &node;
		}
		return nullptr;
	}

	Link* RetrieveLinkFromGroup(Group& group, std::string const&  linkName)
	{
		for (auto& link : group.links)
		{
			if (link.name == linkName)
				return &link;
		}

		LOG_CORE_ERROR("could not find {0} link!!", linkName);
		assert(false);
		return nullptr;
	}

	Parameter* RetrieveParameterFromTree(AnimationTree& tree, std::string const& param_name)
	{
		for (auto& param : tree.parameters)
		{
			if (param.name == param_name)
				return &param;
		}
		return nullptr;
	}

	Timeline* RetrieveTimelineFromAnimation(Animation& animation, std::string const& timelineName)
	{
		for (auto& timeline : animation.timelines)
		{
			if (timeline.name == timelineName)
				return &timeline;
		}
		LOG_CORE_ERROR("could not find {0} timeline in animation!!", timelineName);
		assert(false);
		return nullptr;
	}

	Timeline* TryRetrieveTimelineFromAnimation(Animation& animation, std::string const& timelineName)
	{
		for (auto& timeline : animation.timelines)
		{
			if (timeline.name == timelineName)
				return &timeline;
		}
		return nullptr;
	}

	Parameter* RetrieveParameterFromComponent(IAnimationComponent& comp, std::string const& paramName)
	{
		for (auto& param : comp.tracker.parameters)
		{
			if (param.name == paramName)
			{
				return &param;
			}
		}
		return nullptr;
	}

	Node* AddNodeToGroup(Group& group, Anim::NodeInfo& info)
	{
		//if node already added to this group then just return it
		for (auto& node : group.nodes)
		{
			if (node.name == info.name)
			{
				LOG_CORE_WARN("{0} Animation Node already exists in group!!", info.name);
				return &node;
			}
		}

		//create the node and add it to this group
		Node node{ info };
		//UpdateNodeTrackers(node);
		auto& createdNode = group.nodes.emplace_back(std::move(node));


		//reload node references in case any become invalid
		/*group.startNode.Reload();
		for (auto& link : group.links)
		{
			link.src.Reload();
			link.dst.Reload();
		}*/


		return &createdNode;
	}

	Group* AddGroupToTree(AnimationTree& tree, GroupInfo& info)
	{
		info.tree = &tree;
		Group new_group{ info };
		auto& group = tree.groups.emplace_back(std::move(new_group));
		//create the starting node
		NodeInfo n_info{
			.name{ "Start Node" },
			.animation_name{ Animation::empty_animation_name },
			.speed{ 1.f },
			.position{0.f,0.f,0.f},
			.group{ internal::CreateGroupReference(tree,group.groupID)},
			.nodeID{internal::generateUID() }
		};
		auto node = Anim::internal::AddNodeToGroup(group, n_info);
		assert(node);
		group.startNode = internal::CreateNodeReference(group, n_info.nodeID);

		return &group;
	}

	

	//Node* AddNodeToGroup(AnimationTree& tree, std::string const& groupName, Anim::NodeInfo& info)
	//{
	//	Group* group = RetrieveGroupFromTree(tree, groupName);
	//	if (group == nullptr)
	//	{
	//		LOG_WARN("could not find {0} group to add {1} node!!", groupName, info.name);
	//		return nullptr;
	//	}
	//	//create the node and add it to this group
	//	Node node{*group, name };
	//	return AddNodeToGroup(*group, name, position);
	//}

	Timeline* AddTimelineToAnimation(Animation& animation, Anim::TimelineInfo const& info)
	{
		Timeline* existing_timeline = TryRetrieveTimelineFromAnimation(animation, info.timeline_name);

		//already exists so we just return it
		if (existing_timeline != nullptr)
			return existing_timeline;

		//create the new timeline
		Timeline timeline{ info };
		return &(animation.timelines.emplace_back(std::move(timeline)));
	}

	//Timeline* AddTimelineToAnimation(Animation& animation, std::string const& timelineName, 
	//	Timeline::TYPE type, Timeline::DATATYPE datatype)
	//{
	//	Timeline* existing_timeline = TryRetrieveTimelineFromAnimation(animation, timelineName);

	//	//already exists so we just return it
	//	if (existing_timeline != nullptr)
	//		return existing_timeline;

	//	//create the new timeline
	//	Timeline timeline{ type ,datatype,timelineName };
	//	return &(animation.timelines.emplace_back(std::move(timeline)));
	//}

	bool KeyframeMatchesTimeline(Timeline& timeline, KeyFrame const& keyframe)
	{
		return timeline.rttr_type == keyframe.data.get_type();
		/*switch (timeline.datatype)
		{
		case Timeline::DATATYPE::VEC3:
			return keyframe.data.is_type<glm::vec3>();
			break;
		case Timeline::DATATYPE::QUAT:
			return keyframe.data.is_type<glm::quat>();
			break;
		case Timeline::DATATYPE::BOOL:
			return keyframe.data.is_type<bool>();
			break;
		default:
			return false;
			break;
		}*/
		/*switch (timeline.datatype)	
		{
		case Timeline::DATATYPE::VEC3:
			return std::holds_alternative<glm::vec3>(keyframe.data);
			break;
		case Timeline::DATATYPE::QUAT:
			return std::holds_alternative<glm::quat>(keyframe.data);
			break;
		case Timeline::DATATYPE::BOOL:
			return std::holds_alternative<bool>(keyframe.data);
			break;
		default:
			return false;
			break;
		}*/
	}

	KeyFrame* AddKeyframeToTimeline(Timeline& timeline,KeyFrame const& keyframe)
	{
		assert(KeyframeMatchesTimeline(timeline, keyframe));
		//find index to insert, by finding first keyframe that is further
		size_t index = 0;
		for (auto& kf : timeline.keyframes)
		{
			if (kf.time > keyframe.time)
			{
				break;
			}
			++index;
		}
		//insert
		auto iterator = timeline.keyframes.begin() + index;
		return &( *(timeline.keyframes.emplace(iterator, keyframe)) );
	}

	//node->GetAnimation(), timelineName, type, datatype, keyframe
	//adds a link from src to dst nodes and assumes src and dst nodes are valid
	Link* AddLinkBetweenNodes(Group& group, std::string const& src_name, std::string const& dst_name)
	{
		//links only exist with valid source and destination nodes
		auto* src_node = TryRetrieveNodeFromGroup(group, src_name);
		auto* dst_node = TryRetrieveNodeFromGroup(group, dst_name);

		//source and destination nodes should exist
		assert(src_node != nullptr && dst_node != nullptr);
		auto src_ref = CreateNodeReference(group, src_node->node_ID);
		auto dst_ref = CreateNodeReference(group, dst_node->node_ID);
		Link link{ src_ref , dst_ref };
		link.name = src_node->name + " -> " + dst_node->name;

		auto& createdLink = group.links.emplace_back(std::move(link));

		src_node->outgoingLinks.emplace_back(CreateLinkReference(group, createdLink.linkID));

		return &createdLink;
	}

	Parameter* AddParameterToTree(AnimationTree& tree, Anim::ParameterInfo const& info)
	{
		Parameter param{ info };
		auto& parameter = tree.parameters.emplace_back(std::move(param));
		tree.paramIDtoIndexMap[parameter.paramID] = static_cast<uint>(tree.parameters.size() - 1ull);

		return &parameter;
	}

	Condition* AddConditionToLink(AnimationTree& tree, Link& link, ConditionInfo& info)
	{
		//verify there is a parameter available
		assert(info._param);
		//set the parameter's unique id
		info._paramID = info._param->paramID;

		Condition condition{ info };
		auto& createdCondition = link.conditions.emplace_back(std::move(condition));
		return &createdCondition;
	}

	void LoadFBX(std::string const& filepath, Animation* anim)
	{
		//Assimp::Importer importer;

		//uint flags = 0;
		//flags |= aiProcess_Triangulate;
		//flags |= aiProcess_GenSmoothNormals;
		//flags |= aiProcess_ImproveCacheLocality;
		//flags |= aiProcess_CalcTangentSpace;

		////load the file
		//const aiScene* scene = importer.ReadFile(filepath, flags);
		//if (scene == nullptr)
		//{
		//	LOG_CORE_ERROR("could not load FBX animation from {0}!!", filepath);
		//	assert(false);
		//	return;
		//}

		//if (scene->HasAnimations() == false)
		//{
		//	LOG_CORE_ERROR("could not find any animations in FBX file: {0}!!", filepath);
		//	assert(false);
		//	return;
		//}

		//std::vector<aiAnimation*> fbx_anims;
		//fbx_anims.reserve(scene->mNumAnimations);
		//for (uint i = 0; i < scene->mNumAnimations; i++)
		//{
		//	fbx_anims.emplace_back(scene->mAnimations + i);
		//}



	}



	void InitialiseComponent(IAnimationComponent& comp)
	{
		assert(comp.animTree);
		//copy parameters
		comp.tracker.parameters = comp.animTree->parameters;
		//set current node to start node
		assert(comp.animTree->groups.front().startNode);
		AssignNodeToTracker(comp.tracker, comp.animTree->groups.front().startNode);
	}

	void BindConditionsToParameters(AnimationTree& tree)
	{
		//assign all the parameter's index to the tree's ID to index mapping
		uint index = 0;
		for (auto& param : tree.parameters)
		{
			tree.paramIDtoIndexMap[param.paramID] = index;
			++index;
		}
		//for all conditions
		for (auto& group : tree.groups)
		{
		for (auto& link : group.links)
		{
			for (auto& condition : link.conditions)
			{
				//set param index
				assert(tree.paramIDtoIndexMap.contains(condition.paramID));
				condition.parameterIndex = tree.paramIDtoIndexMap[condition.paramID];

				//set condition comparison function
				internal::AssignComparisonFunctionToCondition(condition);
			}
		}
		}
	}

	void BindNodesToAnimations(AnimationTree& tree)
	{
		for (auto& group : tree.groups)
		{
			for (auto& node : group.nodes)
			{
				node.anim.Reload();
				UpdateNodeTrackers(node);
			}
		}
	}
	

	void CalculateAnimationLength(AnimationTree& tree)
	{
		//for all nodes
		for (auto& group : tree.groups)
		{
			for (auto& node : group.nodes)
			{
				auto& animation = node.GetAnimation();
				float longest_time{ 0.f };

				for (auto& timeline : animation.timelines)
				{
					if (timeline.keyframes.empty()) continue;

					if (longest_time < timeline.keyframes.back().time)
						longest_time = timeline.keyframes.back().time;
				}

				animation.animation_length = longest_time;
			}
		}
	}

	void ReloadReferences(AnimationTree& tree)
	{
		for (auto& group : tree.groups)
		{
			group.startNode.Reload();
			for (auto& node : group.nodes)
			{
				node.group.Reload();
				for (auto& link : node.outgoingLinks)
					link.Reload();
			}
		}
	}

} //namespace oo::Anim::internal

namespace oo::Anim
{
	/*-------------------------------
	NodeRef
	-------------------------------*/
	void NodeRef::Reload()
	{
		int currindex{0};
		for (auto& node : *nodes)
		{
			if (node.node_ID == id)
			{
				index = currindex;
				return;
			}
			++currindex;
		}

		LOG_CORE_ERROR("Animation Node Reference Reload failed!!");
		assert(false);
		index = -1;
	}
	bool NodeRef::valid() const
	{
		return nodes && index >= 0 && index < nodes->size() &&
			this->operator->()->node_ID == id;
	}
	/*-------------------------------
	GroupRef
	-------------------------------*/
	void GroupRef::Reload()
	{
		int currindex{ 0 };
		for (auto& group : *groups)
		{
			if (group.groupID == id)
			{
				index = currindex;
				return;
			}
			++currindex;
		}

		LOG_CORE_ERROR("Animation Group Reference Reload failed!!");
		assert(false);
		index = -1;
	}
	bool GroupRef::valid() const
	{
		return groups && index >= 0 && index < groups->size() &&
			this->operator->()->groupID == id;
	}
	/*-------------------------------
	LinkRef
	-------------------------------*/
	void LinkRef::Reload()
	{
		int currindex{ 0 };
		for (auto& link : *links)
		{
			if (link.linkID == id)
			{
				index = currindex;
				return;
			}
			++currindex;
		}

		LOG_CORE_ERROR("Animation Link Reference Reload failed!!");
		assert(false);
		index = -1;
	}
	bool LinkRef::valid() const
	{
		return links && index >= 0 && index < links->size() &&
			this->operator->()->linkID == id;
	}
	/*-------------------------------
	AnimRef
	-------------------------------*/
	void AnimRef::Reload()
	{
		int currindex{ 0 };
		for (auto& anim : *anims)
		{
			if (anim.animation_ID == id)
			{
				index = currindex;
				return;
			}
			++currindex;
		}

		LOG_CORE_ERROR("Animation Reference Reload failed!!");
		assert(false);
		index = -1;
	}
	bool AnimRef::valid() const
	{
		return anims && index >= 0 && index < anims->size() &&
			this->operator->()->animation_ID == id;
	}
	/*-------------------------------
	Parameter
	-------------------------------*/
	Parameter::Parameter(ParameterInfo const& info) :
		name{info.name}
		, type{ info.type }
		, paramID{ internal::generateUID() }
	{
		
		if (info.value.is_valid() == false)
			value = internal::ParameterDefaultValue(type);
		else
		{
			if (internal::TypeMatchesDataType(this, info.value) == false)
			{
				LOG_CORE_ERROR("Invalid {0} Parameter created!!! Value type different from parameter type!!");
				value = internal::ParameterDefaultValue(type);
				return;
			}

			value = info.value;
		}
	}
	void Parameter::Set(DataType const& _value)
	{
		//value to set should be same type!!
		assert(_value.get_type() != value.get_type());
		value = _value;
	}


	/*-------------------------------
	Condition
	-------------------------------*/
	Condition::Condition(ConditionInfo const& info) 
		: comparison_type{info.comparison}
		, type{ info._param->type }
		//if intial value is set, use that, otherwise use default value based on type 
		, value{info.value.is_valid() ? info.value : internal::ConditionDefaultValue(info._param->type)}
		, paramID{info._paramID}
	{
		//for triggers, only care if trigger boolean is true
		if (type == P_TYPE::TRIGGER)
			value = true;
	}

	bool Condition::Satisfied(AnimationTracker& tracker)
	{
		return internal::ConditionSatisfied(*this, tracker);
	}

	/*-------------------------------
	Link
	-------------------------------*/
	Link::Link(NodeRef _src, NodeRef _dst)
		: src{_src} ,
		dst{_dst} ,
		linkID{ internal::generateUID() }
	{

	}
	/*-------------------------------
	Timeline
	-------------------------------*/
	Timeline::Timeline(TimelineInfo const& info)
		: type{ info.type }
		, rttr_property{info.rttr_property}
		, rttr_type{ info.rttr_property.get_type()}
		, name{info.timeline_name}
		, component_hash{info.component_hash}
		, children_index{ info.children_index }
	{
		get_componentFn = Ecs::ECSWorld::get_component_Fn(component_hash);

		//verify able to retrieve the component info
		assert(get_componentFn != nullptr);
	}
	/*Timeline::Timeline(TYPE _type, DATATYPE _datatype, std::string const _name)
		: type{_type}
		, datatype{_datatype}
		, name{_name}
		, rttr_type{rttr::type::get<TransformComponent>()}
	{
		rttr::property::get_type("")
	}*/


	/*-------------------------------
	Node
	-------------------------------*/
	/*Node::Node(Group& _group, std::string const _name)
		: group{ _group },
		name{ _name }
	{

	}*/

	Node::Node(NodeInfo& info) 
		: group{ (assert(info.group), info.group)}
		, name{ info.name }
		, node_ID{info.nodeID == internal::invalid_ID ? internal::generateUID() : info.nodeID }
	{
		assert(Animation::name_to_index.contains(info.animation_name));
		anim.anims = &(Animation::animation_storage);
		anim.id = Animation::animation_storage[
			Animation::name_to_index[info.animation_name]].animation_ID;
		anim.Reload();
	}

	Animation& Node::GetAnimation()
	{
		assert(anim);
		return *anim;
	}

	/*-------------------------------
	Group
	-------------------------------*/
	/*Group::Group(std::string const _name)
		: name{ _name }
		, groupID{ internal::generateUID() }
	{
		NodeInfo info{
			.name{ "Start Node" },
			.animation_name{ Animation::empty_animation_name },
			.speed{ 1.f },
			.position{0.f,0.f,0.f},
			.group{  }
		};

		auto node = Anim::internal::AddNodeToGroup(*this, info);
		startNode = internal::CreateNodeReference(*this, node->node_ID);
	}*/
	Group::Group(GroupInfo const& info)
		: name{ info.name }
		, groupID{ info.groupID == internal::invalid_ID ? internal::generateUID() : info.groupID }
	{
		//verify tree is valid
		//assert(info.tree);

		
	}

	/*-------------------------------
	AnimationTree
	-------------------------------*/
	std::unordered_map<std::string, AnimationTree> AnimationTree::map;

	AnimationTree* AnimationTree::Create(std::string const name)
	{
		AnimationTree tree;
		tree.name = name;
		AnimationTree::map.emplace(name, std::move(tree));
		auto& createdTree = AnimationTree::map[name];
		//create a default group and assign to tree
		GroupInfo info{ .name{"Group 1"},.tree{&createdTree} };
		internal::AddGroupToTree(AnimationTree::map[name], info);

		return &(AnimationTree::map[name]);
	}


	/*-------------------------------
	ProgressTracker
	-------------------------------*/
	ProgressTracker::ProgressTracker(const Timeline::TYPE _type) :
		type{ _type }
	{

	}
	ProgressTracker ProgressTracker::Create(Timeline::TYPE type)
	{
		ProgressTracker tracker{type};
		switch (type)
		{
		case Timeline::TYPE::PROPERTY:
			tracker.updatefunction = &internal::UpdateProperty_Animation;
			break;
		case Timeline::TYPE::FBX_ANIM:
			tracker.updatefunction = &internal::UpdateFBX_Animation;
			break;
		case Timeline::TYPE::SCRIPT_EVENT:
			tracker.updatefunction = &internal::UpdateEvent;
			break;
		default:
			break;
		}

		return tracker;
	}
	/*-------------------------------
	Animation
	-------------------------------*/
	void Animation::LoadAnimationFromFBX(std::string const& filepath, ModelFileResource* resource)
	{
		/*
		static constexpr auto get_node_hierarchy = [](oGFX::Skeleton const* skeleton, 
			std::string boneName, std::vector<int>& children_index) 
		{
			//skeleton should have nodes!!
			assert(skeleton->m_boneNodes);
			//find the node
			std::stack<oGFX::BoneNode*> stack;
			std::unordered_map<uint32_t, bool> visited{};
			stack.push(skeleton->m_boneNodes);
			visited[skeleton->m_boneNodes->m_BoneIndex] = false;
			oGFX::BoneNode* targetbone{nullptr};
			while (stack.empty() == false)
			{
				auto curr = stack.top();
				stack.pop();

				if (visited[curr->m_BoneIndex] == false)
					visited[curr->m_BoneIndex] = true;

				if (curr->mName == boneName)
				{
					targetbone = curr;
					break;
				}

				for (auto child : curr->mChildren)
				{
					stack.push(child);
					visited[child->m_BoneIndex] = false;
				}
			}
			//we should have found the target bone by now
			assert(targetbone);


		};*/



		Assimp::Importer importer;
		uint flags = 0;
		flags |= aiProcess_Triangulate;
		flags |= aiProcess_GenSmoothNormals;
		flags |= aiProcess_ImproveCacheLocality;
		flags |= aiProcess_CalcTangentSpace;
		flags |= aiProcess_FindInstances; // this step is slow but it finds duplicate instances in FBX
		//flags |= aiProcess_LimitBoneWeights; // limmits bones to 4
		const aiScene* scene = importer.ReadFile(filepath, flags
		);

		if (!scene)
		{
			assert(false);
			//return {}; // Dont explode...
			//throw std::runtime_error("Failed to load model! (" + file + ")");
		}
		assert(scene->HasAnimations());

		//generate hierarchy map
		std::unordered_map<std::string, std::vector<int>> bone_hierarchy_map{};
		{
			std::function<void(oGFX::BoneNode*)> traverse_recursive = [&](oGFX::BoneNode* node)
			{
				if (node->mChildren.empty())
					return;

				int idx = 0;
				std::vector<int> children_index = bone_hierarchy_map[node->mName];
				//add a new element
				children_index.emplace_back(idx);
				for (auto child : node->mChildren)
				{
					//child should not be null!!
					assert(child);
					//set it to the correct index
					children_index.back() = idx;
					//assign it to the map
					bone_hierarchy_map[child->mName] = children_index;
					//recurse
					traverse_recursive(child);
					//increment index
					++idx;
				}

			};

			auto current = resource->skeleton->m_boneNodes;
			std::unordered_map< uint32_t, bool> visited{};
			std::vector<int> children_idx{};
			bone_hierarchy_map[current->mName] = children_idx;
			traverse_recursive(current);
		}


		std::cout << "Animated scene\n";
		for (size_t i = 0; i < scene->mNumAnimations; i++)
		{
			std::cout << "Anim name: " << scene->mAnimations[i]->mName.C_Str() << std::endl;
			std::cout << "Anim frames: " << scene->mAnimations[i]->mDuration << std::endl;
			std::cout << "Anim ticksPerSecond: " << scene->mAnimations[i]->mTicksPerSecond << std::endl;
			std::cout << "Anim duration: " << static_cast<float>(scene->mAnimations[i]->mDuration) / scene->mAnimations[i]->mTicksPerSecond << std::endl;
			std::cout << "Anim numChannels: " << scene->mAnimations[i]->mNumChannels << std::endl;
			std::cout << "Anim numMeshChannels: " << scene->mAnimations[i]->mNumMeshChannels << std::endl;
			std::cout << "Anim numMeshChannels: " << scene->mAnimations[i]->mNumMorphMeshChannels << std::endl;

			Animation anim{};
			anim.name = scene->mAnimations[i]->mName.C_Str();

			for (size_t x = 0; x < scene->mAnimations[i]->mNumChannels; x++)
			{
				auto& channel = scene->mAnimations[i]->mChannels[x];
				//TODO: get node hierarchy and append it to children_index
				oGFX::BoneNode* curr = resource->skeleton->m_boneNodes;
				std::string boneName{ channel->mNodeName.C_Str() };
				//guarding for safety
				assert(resource->strToBone.contains(boneName));
				auto boneindex = resource->strToBone[boneName];
				//bone should exist in the skeleton!!
				assert(bone_hierarchy_map.contains(boneName));
				std::vector<int> children_index = bone_hierarchy_map[boneName];
				//get_node_hierarchy(curr, boneindex, children_index);
				/*--------
				position
				--------*/
				{
					TimelineInfo timeline_info{
					.type{Timeline::TYPE::FBX_ANIM},
					.component_hash{Ecs::ECSWorld::get_component_hash<TransformComponent>()},
					.rttr_property{ rttr::type::get<TransformComponent>().get_property("Position")},
					.timeline_name{channel->mNodeName.C_Str()},
					.children_index = children_index };

					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumPositionKeys; y++)
					{
						auto& key = channel->mPositionKeys[y];
						KeyFrame kf{ .data{ glm::vec3{key.mValue.x,key.mValue.y,key.mValue.z} },
						.time{ static_cast<float>(key.mTime) } };

						auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
					}
				}
				/*--------
				rotation
				--------*/
				{
					TimelineInfo timeline_info{
					.type{Timeline::TYPE::FBX_ANIM},
					.component_hash{Ecs::ECSWorld::get_component_hash<TransformComponent>()},
					.rttr_property{ rttr::type::get<TransformComponent>().get_property("Quaternion")},
					.timeline_name{channel->mNodeName.C_Str()} ,
					.children_index = children_index };

					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumRotationKeys; y++)
					{
						auto& key = channel->mRotationKeys[y];
						KeyFrame kf{ .data{ glm::quat{key.mValue.w, key.mValue.x,key.mValue.y,key.mValue.z} },
						.time{ static_cast<float>(key.mTime) } };

						auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
					}
				}
				/*--------
				scale
				--------*/
				{
					TimelineInfo timeline_info{
					.type{Timeline::TYPE::FBX_ANIM},
					.component_hash{Ecs::ECSWorld::get_component_hash<TransformComponent>()},
					.rttr_property{ rttr::type::get<TransformComponent>().get_property("Scaling")},
					.timeline_name{channel->mNodeName.C_Str()} ,
					.children_index = children_index };

					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumScalingKeys; y++)
					{
						auto& key = channel->mScalingKeys[y];
						KeyFrame kf{ .data{ glm::vec3{key.mValue.x,key.mValue.y,key.mValue.z} },
						.time{ static_cast<float>(key.mTime) } };

						auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
					}
				}
				/*std::cout << "\tKeys name: " << channel->mNodeName.C_Str() << std::endl;
				for (size_t y = 0; y < channel->mNumPositionKeys; y++)
				{
					std::cout << "\t Key_" << std::to_string(y) << " time: " << channel->mPositionKeys[y].mTime << std::endl;
					auto& pos = channel->mPositionKeys[y].mValue;
					std::cout << "\t Key_" << std::to_string(y) << " value: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
				}*/
			}

			auto createdAnim = Animation::AddAnimation(anim);
			assert(createdAnim);
		}
		//std::cout << std::endl;
		
	}
	Animation* Animation::AddAnimation(Animation& anim)
	{
		Animation::ID_to_index[anim.animation_ID] = static_cast<uint>(Animation::animation_storage.size());
		Animation::name_to_index[anim.name] = static_cast<uint>(Animation::animation_storage.size());

		auto& createdAnim = Animation::animation_storage.emplace_back(std::move(anim));

		return &createdAnim;
	}

	/*-------------------------------
	AnimationSystem
	-------------------------------*/
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
			
			internal::UpdateTrackerInfo info{ *this,animationComp.GetActualComponent(),animationComp.GetTracker(), entity,0.016f};
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

		//auto animationfbx_fp = Project::GetAssetFolder().string();
		//animationfbx_fp += "/AnimationTest_Character_IdleJumpAttack.fbx";
		
		//Animation::LoadAnimationFromFBX(animationfbx_fp,);

		auto obj = scene->CreateGameObjectImmediate();
		obj->AddComponent<MeshRendererComponent>();
		auto& comp = obj->AddComponent<oo::AnimationComponent>();
		comp.Set_Root_Entity(obj->GetEntity());

		//create the animation tree asset
		auto tree = AnimationTree::Create("Test Animation Tree");
		comp.SetAnimationTree("Test Animation Tree");
		auto start_node = tree->groups.front().startNode;

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
		auto& group = tree->groups.front();

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
		.timeline_name{"Test Timeline"}
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
		return obj;

	}


	/*-------------------------------
	AnimationComponent
	-------------------------------*/


}

namespace oo
{
	/*-------------------------------
	AnimationComponent Interface
	-------------------------------*/
	Anim::IAnimationComponent& AnimationComponent::GetActualComponent()
	{
		return actualComponent;
	}

	void AnimationComponent::Set_Root_Entity(Ecs::EntityID entity)
	{
		actualComponent.root_objectID = root_object = entity;
	}

	void AnimationComponent::SetAnimationTree(std::string const& name)
	{
		//animTree = _animTree;
		oo::Anim::internal::AssignAnimationTreeToComponent(actualComponent, name);
	}

	void AnimationComponent::SetParameter(std::string const& name, Anim::Parameter::DataType value)
	{
		auto parameter = oo::Anim::internal::RetrieveParameterFromComponent(actualComponent, name);
		assert(parameter);
		assert(value.get_type() == parameter->value.get_type());
		parameter->value = value;
	}

	Anim::AnimationTree* AnimationComponent::GetAnimationTree()
	{
		return actualComponent.animTree;
	}
	Anim::AnimationTracker& AnimationComponent::GetTracker()
	{
		return actualComponent.tracker;
	}

	Anim::Group* AnimationComponent::GetGroup(std::string const& name)
	{
		if (GetAnimationTree() == nullptr)
		{
			LOG_CORE_DEBUG_INFO("No animation tree loaded for this Animation Component!!");
			assert(false);
			return nullptr;
		}

		auto group = oo::Anim::internal::RetrieveGroupFromTree(*GetAnimationTree(), name);

		if (group == nullptr)
			LOG_CORE_DEBUG_INFO("Cannot find {0} group!!", name, GetAnimationTree()->name);

		return group;
	}



	Anim::Node* AnimationComponent::AddNode(std::string const& groupName, Anim::NodeInfo& info)
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
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add node!!", groupName);
			assert(false);
			return nullptr;
		}
		info.group = Anim::internal::CreateGroupReference(*tree, group->groupID);
		auto node = Anim::internal::AddNodeToGroup(*group, info);
		//node should exist after adding to group
		if (node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Adding {0} node to {1} animation tree failed!!", info.name, GetAnimationTree()->name);
			assert(false);
			return nullptr;
		}
		return node;
	}

	Anim::Link* AnimationComponent::AddLink(std::string const& groupName, std::string const& src, std::string const& dst)
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
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add link!!", groupName);
			assert(false);
			return nullptr;
		}

		auto src_node = Anim::internal::RetrieveNodeFromGroup(*group, src);
		auto dst_node = Anim::internal::RetrieveNodeFromGroup(*group, dst);
		//src and dst nodes should exist
		if (src_node == nullptr || dst_node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Link from {0} node to {1} cannot be added!!", src, dst);
			LOG_CORE_DEBUG_INFO("{0} node does not exist!!", ((src_node) ? dst : src) );
			assert(false);
			return nullptr;
		}

		auto link = Anim::internal::AddLinkBetweenNodes(*group, src, dst);

		if (link == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Link from {0} node to {1} cannot be added!!", src, dst);
			assert(false);
		}

		return link;
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
		auto condition = oo::Anim::internal::AddConditionToLink(*tree, *link, info);

		if (condition == nullptr)
		{
			LOG_CORE_DEBUG_INFO("Condition cannot be added to Link {0}!!", linkName);
			assert(false);
		}

		return condition;
	}

	Anim::Timeline* AnimationComponent::AddTimeline(std::string const& groupName, std::string const& nodeName, 
		Anim::TimelineInfo const& info)
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
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add timeline!!", groupName);
			assert(false);
			return nullptr;
		}
		auto node = Anim::internal::RetrieveNodeFromGroup(*group, nodeName);
		//node should exist
		if (node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} node not found, cannot add timeline!!", nodeName);
			assert(false);
			return nullptr;
		}

		//auto timeline = Anim::internal::AddTimelineToAnimation(node->GetAnimation(), timelineName, type, datatype);
		auto timeline = Anim::internal::AddTimelineToAnimation(node->GetAnimation(), info);
		
		if (timeline == nullptr)
		{
			LOG_CORE_DEBUG_INFO("failed to add {0} timeline!!", info.timeline_name);
			assert(false);
			return nullptr;
		}
		//update the node's trackers to reflect the new timeline
		Anim::internal::UpdateNodeTrackers(*node);

		return timeline;
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
			LOG_CORE_DEBUG_INFO("{0} group not found, cannot add timeline!!", groupName);
			assert(false);
			return nullptr;
		}
		auto node = Anim::internal::RetrieveNodeFromGroup(*group, nodeName);
		//node should exist
		if (node == nullptr)
		{
			LOG_CORE_DEBUG_INFO("{0} node not found, cannot add timeline!!", nodeName);
			assert(false);
			return nullptr;
		}
		auto timeline = Anim::internal::RetrieveTimelineFromAnimation(node->GetAnimation(), timelineName);
		//node should exist
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



} //namespace oo
