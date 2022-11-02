/************************************************************************************//*!
\file           AnimationParameter.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A variable referenced by conditions in animation links between nodes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"
#include "AnimationGroup.h"
namespace oo::Anim
{
	//variables that are defined within an AnimationTree that can be accessed and assigned values from scripts or editor
	struct Parameter
	{
		using DataType = rttr::variant;

		P_TYPE type{};
		DataType value{};
		size_t paramID{ internal::invalid_ID };
		std::string name{ "Unnamed Parameter" };

		Parameter() = default;
		Parameter(ParameterInfo const& info);
		void Set(DataType const& _value);

		RTTR_ENABLE();
	};

	struct ParameterInfo
	{
		std::string name{ "Unnamed Parameter" };
		P_TYPE type;
		//optional initial value
		Parameter::DataType value;
	};
}