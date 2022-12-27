/************************************************************************************//*!
\file           AnimationSkeleton.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          An collection of information about the skeletal mesh and the bones and 
				other related information to be used for animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "AnimationPose.h"

namespace oo::Anim::internal
{
}

namespace oo::Anim
{
	class Skeleton
	{

	private:
		Pose starting_pose{};
	};
}
