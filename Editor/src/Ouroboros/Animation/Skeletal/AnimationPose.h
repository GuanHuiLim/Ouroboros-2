/************************************************************************************//*!
\file           AnimationPose.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          An animation pose

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "../Anim_Utils.h"

#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
namespace oo::Anim::internal
{
	constexpr static uint MAX_HIERARCHY_NUM = 10;
}

namespace oo::Anim
{
	struct Bone
	{
		Ecs::EntityID id{};
		glm::mat4 transform{};
		std::array<int, internal::MAX_HIERARCHY_NUM> hierarchy{};

	};

	class Pose
	{
		Bone rootBone{};
		std::vector<Bone> bones{};

		float time{};
	};
}

//internal functions
namespace oo::Anim::internal
{

}