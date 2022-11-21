/************************************************************************************//*!
\file           AnimationGroup.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A collection of nodes and links

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"
#include "AnimationNode.h"
#include "AnimationLink.h"
namespace oo::Anim
{
	struct Group
	{
		using SerializeFn = void(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>&, Group&);
		static SerializeFn* const serializeFn;

		std::string name{ "Unnamed Group" };
		NodeRef startNode{};
		NodeRef any_state_Node{};
		//contains the nodes and their positions to be displayed in the editor
		std::map<size_t, Node> nodes{};
		std::map<size_t, Link> links{};
		//AnimationTree* tree{ nullptr };
		size_t groupID{ internal::invalid_ID };	//unique identifier
		

		//Group(std::string const _name = "Unnamed Group");
		Group() = default;
		Group(GroupInfo const& info);
		Group(Group&& other);
		//Group(Group const&) = default;

		RTTR_ENABLE();
	};

	struct GroupInfo
	{
		std::string name{ "Unnamed Group" };
		size_t groupID{ internal::invalid_ID };
		AnimationTree* tree{ nullptr };
	};
}