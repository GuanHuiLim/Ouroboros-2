/************************************************************************************//*!
\file          NavAgentComponent.h
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 19, 2022
\brief         File contains the declaration for NavAgentComponent

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <Quaternion/include/Transform.h>

#include <rttr/type>
#include <vector>

namespace oo
{
	class NavAgentComponent
	{
		using Vector3 = Transform::vec3;
	public:
		float speed = 20.0f;
		//NavGraphComponent grid;		//requires graph
		bool drawGizmos = false;
		bool reached = false;
		Vector3* target;
		Vector3 direction;
		int targetIndex = 0;
		std::vector<Vector3*> currPath;

		RTTR_ENABLE();

	private:
		float currentSpeed;

		friend class NavSystem;
	};
}
