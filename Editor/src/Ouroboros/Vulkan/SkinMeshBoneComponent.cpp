#include "pch.h"
#include "RendererComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>

namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
	registration::class_<SkinMeshBoneComponent>("SkinMeshBoneComponent")
		.property("bone_name", &SkinMeshBoneComponent::bone_name);
	};
}
