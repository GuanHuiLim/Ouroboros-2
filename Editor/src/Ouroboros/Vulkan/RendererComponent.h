#pragma once
#include "pch.h"
#include "glm/common.hpp"
#include "Ouroboros/Asset/AssetManager.h"
#include "OO_Vulkan/src/MeshModel.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "OO_Vulkan/src/DefaultMeshCreator.h"
namespace oo
{
	class Material;


	

	struct MeshRendererComponent
	{
		Asset mesh_handle;

		//no need to serialize
		uint32_t model_handle;
		uint32_t graphicsWorld_ID;

		void GetModelHandle()
		{
			model_handle = mesh_handle.GetData<Model>().gfxIndex;
		}

		//std::vector<Material> materials;

		//Lighting

		//Probes

		//Additional Settings
	};

	struct SkinMeshRendererComponent
	{
		Asset mesh_handle;
	};

}

namespace oo::Mesh
{
	auto CreateCubeMeshObject(Scene* scene, GraphicsWorld* graphicsWorld)
	{
		//auto cubeMesh = CreateDefaultCubeMesh();

		auto gameObj = scene->CreateGameObjectImmediate();
		auto& meshRenderer = gameObj->AddComponent<MeshRendererComponent>();
		meshRenderer.graphicsWorld_ID = graphicsWorld->CreateObjectInstance();
		meshRenderer.model_handle = 0;
		//meshRenderer.graphicsWorld_ID = graphicsWorld->CreateObjectInstance();

		return gameObj;
	}
	/*GameObject CreateMeshObject( Scene* scene, std::string filepath )
	{

	}*/
}
