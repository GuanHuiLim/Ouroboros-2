/************************************************************************************//*!
\file          AnimatorControllerView.h
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 22, 2022
\brief         File Contains the declaration needed to create an Animator Controller View
			   for the engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_node_editor.h>
#include <imgui/imgui_internal.h>
#include <vector>
#include <string>
#include <Ouroboros/Animation/Anim.h>
#include <Ouroboros/Animation/AnimationComponent.h>

namespace ed = ax::NodeEditor;

constexpr ImGuiID popUpOptionId = 600;

class AnimatorControllerView
{
public:	//Default Functions
	AnimatorControllerView() 
	{
		m_Context = ed::CreateEditor();
	}
	~AnimatorControllerView() 
	{
		ed::DestroyEditor(m_Context);
	}

	void Show();

private: //Member Variables
	oo::AnimationComponent* animator = nullptr;
	ed::EditorContext* m_Context	  = nullptr;

	enum class PinKind
	{
		Output,
		Input
	};

	struct NodeInfo;

	struct Pin
	{
		ed::PinId	id;
		NodeInfo*   node;
		std::string name;
		PinKind		kind;

		Pin(uintptr_t _id, NodeInfo* _node, const char* _name):
			id(_id), node(_node), name(_name), kind(PinKind::Input)
		{
		}
	};

	struct NodeInfo
	{
		ed::NodeId id;
		oo::Anim::Node* anim_node;
		std::vector<Pin> Input;
		std::vector<Pin> Output;
		ImColor color;
		ImVec2 size;
		ImVec2 pos;
		bool selected;

		std::string state;
		std::string savedState;

		NodeInfo(/*uintptr_t _id,*/
				 oo::Anim::Node* _anim_node = nullptr,
				 ImColor _color = ImColor(255, 255, 255)) 
		:id(static_cast<void*>(_anim_node)),
		anim_node{ _anim_node },
		color(_color), 
		size(0, 0), 
		pos(0, 0), 
		selected(false)
		{

		}
	};
	std::vector<NodeInfo> m_nodes;
	std::vector<ed::NodeId> m_nodesId;	//used for displaying onto animator node inspector
	std::vector<oo::Anim::Node*> m_newNodes;

	struct LinkInfo
	{
		ed::LinkId id;
		oo::Anim::Link* link;
		ed::PinId inputID;
		ed::PinId outputID;
		bool selected;

		LinkInfo(/*ed::LinkId _id,*/ 
				 ed::PinId _inputID = {}, 
				 ed::PinId _outputID = {}, 
				 oo::Anim::Link* _link = nullptr) 
		:id(static_cast<void*>(_link)),
		inputID(_inputID), 
		outputID(_outputID), 
		selected(false), 
		link{_link}
		{
		}
	};
	std::vector<LinkInfo> m_links_;
	std::vector<ed::LinkId> m_linksId;	//used for displaying onto animator link inspector

	bool m_firstFrame				  = true;

	uintptr_t uniqueId				  = 1;		//for debugging purposes
	int  m_nextLinkId				  = 100;	//for debugging purposes

	std::stack<uintptr_t> free_Node_IDs{};		//track free ids

	std::string current_group_name{};
private: //Member Functions

	void BuildNode(NodeInfo* node);

	void DisplayAnimatorController(oo::AnimationComponent* _animator);
	void DisplayParameters();
	void DisplayInspector();
	void DisplayConditions(oo::Anim::Link* link);
	NodeInfo* CreateNode(oo::Anim::Node* _anim_node);
	LinkInfo* CreateLink(oo::Anim::Link* link, ed::PinId inputPinId, ed::PinId outputPinId);
	void OnDelete();

	void PostDisplayUpdate(oo::AnimationComponent* _animator);
	//Helper Functions
	Pin* FindPin(ed::PinId id);
	NodeInfo* FindNode(ed::PinId pinID);
	NodeInfo* FindNode(ed::NodeId id);
	NodeInfo* FindNode(oo::Anim::Node* _node);

	//managing node unique IDs
	void ReturnID(ed::NodeId id);
	void ReturnID(ed::PinId id);
	void ReturnID(ed::LinkId id);
	uintptr_t GetAvailableNodeID();

	//loads data from AnimationComponent
	void LoadGraph(oo::AnimationComponent* _animator);
	//void BuildNodes();
	void BuildNodeOnEditor(NodeInfo& info);
	void Clear();
};
