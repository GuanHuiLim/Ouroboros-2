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
	struct SkinMeshRendererComponent
	{
		Asset mesh_handle;

		//no need to serialize
		int gfxMeshIndices_index{-1};
		uint32_t gfxMeshIndice{ 0 };
		uint32_t graphicsWorld_ID;

		RTTR_ENABLE();
	};

	struct SkinMeshBoneComponent
	{
		std::string bone_name{};
		uint32_t gfxbones_index{ 
			std::numeric_limits<decltype(gfxbones_index)>().max() };
		uint32_t graphicsWorld_ID;

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
