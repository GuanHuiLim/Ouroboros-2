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

namespace ed = ax::NodeEditor;

constexpr ImGuiID popUpOptionId = 600;

class AnimatorControllerView
{
public:	//Default Functions
	AnimatorControllerView() 
	{
		CreateNode(uniqueId, "Entry", ImVec2(500, 100));
		CreateNode(uniqueId, "Exit", ImVec2(800, 100));
		CreateNode(uniqueId, "Any State", ImVec2(500, 0));
		m_Context = ed::CreateEditor();
	}
	~AnimatorControllerView() 
	{
		ed::DestroyEditor(m_Context);
	}

	void Show();

private: //Member Variables
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

		Pin(int _id, NodeInfo* _node, const char* _name):
			id(_id), node(_node), name(_name), kind(PinKind::Input)
		{
		}
	};
	std::vector<Pin> m_pins;	//to be used to track which pins are used for links

	struct NodeInfo
	{
		ed::NodeId id;
		std::string name;
		std::vector<Pin> Input;
		std::vector<Pin> Output;
		ImColor color;
		ImVec2 size;
		ImVec2 pos;
		bool selected;

		std::string state;
		std::string savedState;

		//AnimatorNode* anim_node;	//basically a way to get the animator information from

		NodeInfo(int _id, const char* _name, ImColor _color = ImColor(255, 255, 255)) :
			id(_id), name(_name), color(_color), size(0, 0), selected(false)
		{
		}
	};
	std::vector<NodeInfo> m_nodes;
	std::vector<ed::NodeId> m_nodesId;
	NodeInfo* m_selectedNode		  = nullptr;

	struct LinkInfo
	{
		ed::LinkId id;
		ed::PinId inputID;
		ed::PinId outputID;
		bool selected;

		LinkInfo(ed::LinkId _id, ed::PinId _inputID, ed::PinId _outputID) :
			id(_id), inputID(_inputID), outputID(_outputID), selected(false)
		{
		}
	};
	ImVector<LinkInfo> m_links;
	std::vector<ed::LinkId> m_linksId;
	LinkInfo* m_selectedLink		  = nullptr;

	bool m_firstFrame				  = true;

	int uniqueId					  = 1;
	int  m_nextLinkId				  = 100;	//for debugging purposes

private: //Member Functions

	void BuildNode(NodeInfo* node);
	void BuildNodes();

	void DisplayAnimatorController();
	void DisplayParameters();
	void DisplayInspector();
	void OnCreate();
	NodeInfo* CreateNode(int& uniqueId, const char* _name, ImVec2 _pos = ImVec2(0,0));
	void CreateLink(int& uniqueId);
	void OnDelete();
	void DeleteNode(int& uniqueId);
	void DeleteLink(int& uniqueId);
	void RightClickOptions(int& id);

	//Helper Functions
	Pin* FindPin(ed::PinId id);
};
