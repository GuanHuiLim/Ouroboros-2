#include "pch.h"
#include "SkinRendererComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>

namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<SkinMeshRendererComponent>("Skinned Mesh Renderer Component")
		.property_readonly("Model Handle", &SkinMeshRendererComponent::meshResource)
		.property_readonly("Number of bones", &SkinMeshRendererComponent::num_bones)
		.property("Albedo", &SkinMeshRendererComponent::GetAlbedo, &SkinMeshRendererComponent::SetAlbedo)
		(
			metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
		)
		.property("Normal", &SkinMeshRendererComponent::GetNormal, &SkinMeshRendererComponent::SetNormal)
		(
			metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
		)
		.property("Mesh", &SkinMeshRendererComponent::GetMesh, &SkinMeshRendererComponent::SetMesh)
		(
			metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Model))
		)
			
		;
	};


	void SkinMeshRendererComponent::SetAlbedo(Asset albedo)
	{
		albedo_asset = albedo;
		if (albedo_asset.IsValid())
		{
			albedoID = albedo_asset.GetData<uint32_t>();
		}
	}
	Asset SkinMeshRendererComponent::GetAlbedo() const
	{
		return albedo_asset;
	}
	void SkinMeshRendererComponent::SetNormal(Asset normal)
	{
		normal_asset = normal;
		if (normal_asset.IsValid())
		{
			normalID = normal_asset.GetData<uint32_t>();
		}
	}
	Asset SkinMeshRendererComponent::GetNormal() const
	{
		return normal_asset;
	}

	Asset SkinMeshRendererComponent::GetMesh()
	{
		return mesh_asset;
	}

	void SkinMeshRendererComponent::SetMesh(Asset asset)
	{
		if (asset.IsValid())
		{
			mesh_asset = asset;
			meshResource = mesh_asset.GetData<ModelFileResource*>()->meshResource;
			//ensure skeleton exists
			assert(mesh_asset.GetData<ModelFileResource*>()->skeleton);
			num_bones = mesh_asset.GetData<ModelFileResource*>()->skeleton->inverseBindPose.size();
		}
	}
	void oo::SkinMeshRendererComponent::SetModelHandle(Asset _asset, uint32_t _submodel_id)
	{
		meshInfo.submeshBits.reset();
		meshInfo.submeshBits[_submodel_id] = true;
		meshInfo.mesh_handle = _asset;

		meshResource = meshInfo.mesh_handle.GetData<ModelFileResource*>()->meshResource;
	}
	MeshInfo oo::SkinMeshRendererComponent::GetMeshInfo()
	{
		return meshInfo;
	}
	void oo::SkinMeshRendererComponent::SetMeshInfo(MeshInfo info)
	{
		meshInfo.submeshBits = info.submeshBits;
	}
}
