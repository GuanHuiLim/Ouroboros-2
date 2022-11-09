/************************************************************************************//*!
\file           AnimationTree.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
allows you to arrange and maintain a set of Animations and 
Animation Transitions for a gameobject object

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
		friend class AnimationSystem;
		static std::unordered_map<size_t, AnimationTree> map;
	
		std::string name{"Unnamed Animation Tree"};
		//contains a collection of nodes and links between the nodes 
		std::map<size_t, Group> groups;
		//contains a collection of parameters to be checked with conditions
		std::vector<Parameter> parameters;
		std::unordered_map<size_t, uint> paramIDtoIndexMap;
		//std::vector<Animation> animations;
		//std::vector<Node> nodes;
		size_t treeID{ internal::invalid_ID };

	
		RTTR_ENABLE();

	private:
		static AnimationTree* Create(std::string const name = "Unnamed Animation Tree");
		static AnimationTree* Add(AnimationTree&& tree);
	};


	
}