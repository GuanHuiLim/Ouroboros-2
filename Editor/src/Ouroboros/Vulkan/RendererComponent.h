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

#include <rttr/type>
namespace oo
{
	class Material;

	struct MeshRendererComponent
	{
		Asset albedo_handle;
		Asset normal_handle;
		Asset mesh_handle;
		int submodel_id = 0;
		uint32_t albedoID = 0xFFFFFFFF;
		uint32_t normalID = 0xFFFFFFFF;

		//no need to serialize
		uint32_t model_handle{0};
		uint32_t graphicsWorld_ID{};

		void GetModelHandle()
		{
			if (mesh_handle.HasData())
				model_handle = mesh_handle.GetData<ModelData*>()->gfxMeshIndices[submodel_id];
			else
				model_handle = 0;
			
			//model_handle /*= mesh_handle.GetData<ModelData>().gfxMeshIndices.front()*/;
		}
		void SetModelHandle(Asset _asset, uint32_t _submodel_id)
		{
			submodel_id = _submodel_id;
			mesh_handle = _asset;

			model_handle = mesh_handle.GetData<ModelData*>()->gfxMeshIndices[submodel_id];
		}
		Asset GetMesh()
		{
			return mesh_handle;
		}
		void SetMesh(Asset _asset)
		{
			if (_asset.IsValid())
			{
				mesh_handle = _asset;
				model_handle = mesh_handle.GetData<ModelData*>()->gfxMeshIndices[submodel_id];
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
		int GetSubModelID()
		{
			return submodel_id;
		}
		void SetSubModelID(int id)
		{
			if (mesh_handle.IsValid())
			{
				auto modelData = mesh_handle.GetData<ModelData*>();
				id = id %  modelData->gfxMeshIndices.size();
				submodel_id = id %  modelData->gfxMeshIndices.size();
				model_handle = modelData->gfxMeshIndices[submodel_id];
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
