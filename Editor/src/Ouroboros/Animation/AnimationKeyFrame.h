/************************************************************************************//*!
\file           AnimationKeyFrame.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
Keyframes of an animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"

namespace oo::Anim
{
	struct KeyFrame
	{
		//position & scale vec3, rotation quat
		//using DataType = std::variant<glm::vec3,glm::quat,bool>;
		using DataType = rttr::variant;
		//vector3 variable HERE
		DataType data;
		float time{ 0.f };

		KeyFrame(DataType _data, float _time);
		RTTR_ENABLE();
	};

	struct ScriptEvent
	{
		//scriptevent handle or something variable HERE
		oo::ScriptValue::function_info script_function_info{};
		float time{ 0.f };

		RTTR_ENABLE();
	};

}