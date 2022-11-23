#pragma once
#include "pch.h"
#ifndef  GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define  GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif // ! GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include "glm/common.hpp"
#include "Ouroboros/Asset/AssetManager.h"
#include "OO_Vulkan/src/MeshModel.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "OO_Vulkan/src/DefaultMeshCreator.h"
#include "OO_Vulkan/src/GraphicsWorld.h"
#include "MeshInfo.h"
#include "Utility/UUID.h"
#include <rttr/type>
namespace oo
{
	struct SkinMeshRendererComponent
	{
		Asset albedo_asset;
		Asset normal_asset;
		
		Asset mesh_asset;
		MeshInfo meshInfo;

		uint32_t albedoID = 0xFFFFFFFF;
		uint32_t normalID = 0xFFFFFFFF;
		
		//no need to serialize
		uint32_t graphicsWorld_ID{};
		uint32_t meshResource{ 0 };
		size_t num_bones{ 0 };

		//temporary data, do not touch
		ObjectInstance* gfx_Object{ nullptr };


		void SetAlbedo(Asset albedo);
		Asset GetAlbedo() const;

		void SetNormal(Asset normal);
		Asset GetNormal() const;

		Asset GetMesh();
		void SetMesh(Asset asset);

		void SetModelHandle(Asset _asset, uint32_t _submodel_id);

		MeshInfo GetMeshInfo();
		void SetMeshInfo(MeshInfo info);

		RTTR_ENABLE();
	};

	struct SkinMeshBoneComponent
	{
		std::string bone_name{};
		//uint32_t gfxbones_index{ std::numeric_limits<decltype(gfxbones_index)>().max() };
		oGFX::BoneInverseBindPoseInfo inverseBindPose_info{};
		uint32_t graphicsWorld_ID;
		glm::mat4 globalTransform{};
		UUID skin_mesh_object{};

		void SetInverseBindPoseInfo_BoneIdx(uint32_t boneIdx);
		uint32_t GetInverseBindPoseInfo_BoneIdx();

		void SetInverseBindPoseInfo_Transform_X(glm::vec4 transform);
		glm::vec4 GetInverseBindPoseInfo_Transform_X();


		void SetInverseBindPoseInfo_Transform_Y(glm::vec4 transform);
		glm::vec4 GetInverseBindPoseInfo_Transform_Y();

		void SetInverseBindPoseInfo_Transform_Z(glm::vec4 transform);
		glm::vec4 GetInverseBindPoseInfo_Transform_Z();

		void SetInverseBindPoseInfo_Transform_W(glm::vec4 transform);
		glm::vec4 GetInverseBindPoseInfo_Transform_W();

		void SetSkinMeshObjectUUID(size_t uid);
		size_t GetSkinMeshObjectUUID();

		RTTR_ENABLE();
	};

}

// testing code i assume
//namespace oo::Mesh
//{
//	auto CreateCubeMeshObject(Scene* scene, GraphicsWorld* graphicsWorld)
//	{
//		//auto cubeMesh = CreateDefaultCubeMesh();
//
//		auto gameObj = scene->CreateGameObjectImmediate();
//		auto& meshRenderer = gameObj->AddComponent<MeshRendererComponent>();
//		meshRenderer.graphicsWorld_ID = graphicsWorld->CreateObjectInstance();
//		meshRenderer.model_handle = 0;
//		//meshRenderer.graphicsWorld_ID = graphicsWorld->CreateObjectInstance();
//
//		return gameObj;
//	}
//}
