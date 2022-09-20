/************************************************************************************//*!
\file          NavGraphComponent.cpp
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 19, 2022
\brief         File contains the implementation to register the data of NavGraphComponent

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "NavGraphComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>

namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<NavGraphComponent>("NavGraph")
			.property("Grid World Size", &NavGraphComponent::gridWorldSize)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
			.property("Node Radius", &NavGraphComponent::nodeRadius)
			.property("Node Height", &NavGraphComponent::nodeHeight)
			.property("Draw Gizmos", &NavGraphComponent::drawGizmos);
	}
}
