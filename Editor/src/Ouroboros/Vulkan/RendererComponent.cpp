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
		.property("Albedo", &MeshRendererComponent::albedo_handle)
		(
			metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
		)
		.property("Normal", &MeshRendererComponent::normal_handle)
		(
			metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
		)
		.property("Mesh", &MeshRendererComponent::GetMesh, &MeshRendererComponent::SetMesh)
		(
			metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Model))
		)
		.property("MeshInfo", &MeshRendererComponent::GetMeshInfo, &MeshRendererComponent::SetMeshInfo);
	}
}
