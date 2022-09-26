/************************************************************************************//*!
\file           AnimationTree.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          BRIEF_HERE

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"

#include <unordered_map>
namespace oo::Anim
{
	struct AnimationTree
	{
		static std::unordered_map<std::string, AnimationTree> map;
	
		std::string name{"Unnamed Animation Tree"};
		//contains a collection of nodes and links between the nodes 
		std::vector<Group> groups;
		//contains a collection of parameters to be checked with conditions
		std::vector<Parameter> parameters;
		std::unordered_map<size_t, uint> paramIDtoIndexMap;
		std::vector<Animation> animations;
		std::vector<Node> nodes;

		static AnimationTree* Create(std::string const name = "Unnamed Animation Tree");
	};


	
}