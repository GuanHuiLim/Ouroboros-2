/************************************************************************************//*!
\file           AnimationLink.h
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
	struct Link
	{
		using SerializeFn = void(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>&, Link&);
		static SerializeFn*  serializeFn;


		NodeRef src;
		NodeRef dst;

		bool has_exit_time{ false };
		float exit_time{ 0.f };
		bool fixed_duration{ false };
		float transition_duration{ 0.f };
		float transition_offset{ 0.f };
		std::string name{ "Unnamed Link" };
		std::vector<Condition> conditions{};
		size_t linkID{ internal::invalid_ID };

		Link() = default;
		Link(NodeRef _src, NodeRef _dst);

		RTTR_ENABLE();
	};


}