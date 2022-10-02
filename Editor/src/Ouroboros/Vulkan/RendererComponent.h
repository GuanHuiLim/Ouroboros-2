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
#include "MeshInfo.h"
#include <bitset>
#include <rttr/type>
namespace oo
{
	class Material;

	struct MeshRendererComponent
	{
		Asset albedo_handle;
		Asset normal_handle;
		MeshInfo meshInfo;
		uint32_t albedoID = 0xFFFFFFFF;
		uint32_t normalID = 0xFFFFFFFF;

		//no need to serialize
		uint32_t model_handle{0};
		uint32_t graphicsWorld_ID{};

		MeshInfo GetMeshInfo() 
		{
			return meshInfo;
		}
		/*********************************************************************************//*!
		\brief      this function will only set the submeshbits
		*//**********************************************************************************/
		void SetMeshInfo(MeshInfo info) 
		{
			meshInfo.submeshBits = info.submeshBits;
		}

		void GetModelHandle()
		{
			if (meshInfo.mesh_handle.HasData())
				model_handle = meshInfo.mesh_handle.GetData<ModelFileResource*>()->meshResource;
			else
				model_handle = 0;
			
			//model_handle /*= meshInfo.mesh_handle.GetData<ModelFileResource>().meshResource*/;
		}
		
		//set a single model and asset
		void SetModelHandle(Asset _asset, uint32_t _submodel_id)
		{
			meshInfo.submeshBits.reset();
			meshInfo.submeshBits[_submodel_id] = true;
			meshInfo.mesh_handle = _asset;

			model_handle = meshInfo.mesh_handle.GetData<ModelFileResource*>()->meshResource;
		}
		Asset GetMesh()
		{
			return meshInfo.mesh_handle;
		}
		void SetMesh(Asset _asset)
		{
			if (_asset.IsValid())
			{
				meshInfo.mesh_handle = _asset;
				model_handle = meshInfo.mesh_handle.GetData<ModelFileResource*>()->meshResource;
				// HACK this is needed to render stuff under edit..
				// meshInfo.submeshBits.reset();
				// meshInfo.submeshBits[0] = true;
			}
			if (albedo_handle.IsValid())
			{
				albedoID = albedo_handle.GetData<uint32_t>();
			}
			if (normal_handle.IsValid())
			{
				normalID = normal_handle.GetData<uint32_t>();
			}
		}	   

		

		//std::vector<Material> materials;

		//Lighting

		//Probes

		//Additional Settings
		RTTR_ENABLE();
	};

	struct SkinMeshRendererComponent
	{
		Asset mesh_handle;


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
