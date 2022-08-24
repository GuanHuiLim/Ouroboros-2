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
	}

	void PrefabComponent::Init(std::filesystem::path& p)
	{
		prefab_filePath = p;
	}


};
