#pragma once
#include "pch.h"
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
		Asset mesh_handle;

		//no need to serialize
		uint32_t submodel_id = 0;
		uint32_t model_handle{0};
		uint32_t graphicsWorld_ID;

		void GetModelHandle()
		{
			model_handle = mesh_handle.GetData<ModelData*>()->gfxMeshIndices[submodel_id];
			//model_handle /*= mesh_handle.GetData<ModelData>().gfxMeshIndices.front()*/;
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
