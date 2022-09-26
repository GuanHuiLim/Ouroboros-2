/************************************************************************************//*!
\file           PrefabComponent.cpp
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          simple component for the root prefab object to contain 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "PrefabComponent.h"
#include <rttr/registration.h>

namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<PrefabComponent>("Prefab Component")
		.property("File Path", &PrefabComponent::prefab_filePath);
	}
	PrefabComponent::PrefabComponent()
	{
	}

	PrefabComponent::~PrefabComponent()
	{
		prefab_filePath.clear();
	}

	void PrefabComponent::Init(std::filesystem::path& p)
	{
		prefab_filePath = p;
	}


};
