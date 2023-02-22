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
		.property_readonly("Picking ID", &SkinMeshRendererComponent::picking_ID)
		.property("Number of bones", &SkinMeshRendererComponent::num_bones)
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
		.property("MeshInfo", &SkinMeshRendererComponent::GetMeshInfo, &SkinMeshRendererComponent::SetMeshInfo)
		.property("Cast Shadows",    &SkinMeshRendererComponent::CastShadows)
		.property("Receive Shadows", &SkinMeshRendererComponent::ReceiveShadows)
		;

		registration::class_<SkinMeshBoneComponent>("Skinned Mesh Bone Component")
		.property("bone_name", &SkinMeshBoneComponent::bone_name)
		.property("inverseBindPose_info->boneIdx",	 &SkinMeshBoneComponent::GetInverseBindPoseInfo_BoneIdx, &SkinMeshBoneComponent::SetInverseBindPoseInfo_BoneIdx)
		.property("inverseBindPose_info->transform.x", &SkinMeshBoneComponent::GetInverseBindPoseInfo_Transform_X, &SkinMeshBoneComponent::SetInverseBindPoseInfo_Transform_X)
		.property("inverseBindPose_info->transform.y", &SkinMeshBoneComponent::GetInverseBindPoseInfo_Transform_Y, &SkinMeshBoneComponent::SetInverseBindPoseInfo_Transform_Y)
		.property("inverseBindPose_info->transform.z", &SkinMeshBoneComponent::GetInverseBindPoseInfo_Transform_Z, &SkinMeshBoneComponent::SetInverseBindPoseInfo_Transform_Z)
		.property("inverseBindPose_info->transform.w", &SkinMeshBoneComponent::GetInverseBindPoseInfo_Transform_W, &SkinMeshBoneComponent::SetInverseBindPoseInfo_Transform_W)
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
		return meshInfo.mesh_handle;
	}

	void SkinMeshRendererComponent::SetMesh(Asset asset)
	{
		if (asset.IsValid())
		{
			meshInfo.mesh_handle = asset;
			meshResource = meshInfo.mesh_handle.GetData<ModelFileResource*>()->meshResource;

			//ensure skeleton exists
			assert(meshInfo.mesh_handle.GetData<ModelFileResource*>()->skeleton);
			num_bones = meshInfo.mesh_handle.GetData<ModelFileResource*>()->skeleton->inverseBindPose.size();
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


	void SkinMeshBoneComponent::SetInverseBindPoseInfo_BoneIdx(uint32_t boneIdx)
	{
		inverseBindPose_info.boneIdx = boneIdx;

	}

	uint32_t SkinMeshBoneComponent::GetInverseBindPoseInfo_BoneIdx()
	{
		return inverseBindPose_info.boneIdx;
	}


	void SkinMeshBoneComponent::SetInverseBindPoseInfo_Transform_X(glm::vec4 transform)
	{
		inverseBindPose_info.transform[0] = transform;
	}
	glm::vec4 SkinMeshBoneComponent::GetInverseBindPoseInfo_Transform_X()
	{
		return inverseBindPose_info.transform[0];
	}

	void SkinMeshBoneComponent::SetInverseBindPoseInfo_Transform_Y(glm::vec4 transform)
	{
		inverseBindPose_info.transform[1] = transform;
	}
	glm::vec4 SkinMeshBoneComponent::GetInverseBindPoseInfo_Transform_Y()
	{
		return inverseBindPose_info.transform[1];
	}

	void SkinMeshBoneComponent::SetInverseBindPoseInfo_Transform_Z(glm::vec4 transform)
	{
		inverseBindPose_info.transform[2] = transform;
	}
	glm::vec4 SkinMeshBoneComponent::GetInverseBindPoseInfo_Transform_Z()
	{
		return inverseBindPose_info.transform[2];
	}

	void SkinMeshBoneComponent::SetInverseBindPoseInfo_Transform_W(glm::vec4 transform)
	{
		inverseBindPose_info.transform[3] = transform;
	}
	glm::vec4 SkinMeshBoneComponent::GetInverseBindPoseInfo_Transform_W()
	{
		return inverseBindPose_info.transform[3];
	}

	void SkinMeshBoneComponent::SetSkinMeshObjectUUID(size_t uid)
	{
		root_bone_object = uid;
	}
	size_t SkinMeshBoneComponent::GetSkinMeshObjectUUID()
	{
		return root_bone_object;
	}
}
