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
#include "Anim_Utils.h"

#include <Archetypes_ECS/src/A_Ecs.h>
namespace oo::Anim::internal
{

}

namespace oo::Anim
{
	struct Bone
	{
		Ecs::EntityID id{};
		glm::mat4 transform{};
	};

	class Pose
	{
		Bone rootBone{};

		std::vector<Bone> bones{};
	};
}
