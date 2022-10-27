#include "pch.h"
#include "RendererComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>

namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<MeshRendererComponent>("Mesh Renderer")
		.property_readonly("Model Handle", &MeshRendererComponent::model_handle)
		.property("Mesh", &MeshRendererComponent::mesh_handle)
			(
				metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
			);
	};

}
