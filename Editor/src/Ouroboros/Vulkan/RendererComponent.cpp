/************************************************************************************//*!
\file           RendererComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Sept 30, 2022
\brief          Describes the Mesh Renderer and Skin Mesh Renderer Components that'll
                be the interface for users to fill up and used by renderer system.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "RendererComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>
#include <glm/common.hpp>

#include "OO_Vulkan/src/MeshModel.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "OO_Vulkan/src/DefaultMeshCreator.h"

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
	
	void MeshRendererComponent::GetModelHandle()
	{
		if (mesh_handle.HasData())
			model_handle = mesh_handle.GetData<ModelData*>()->gfxMeshIndices[submodel_id];
		else
			model_handle = 0;

		//model_handle /*= mesh_handle.GetData<ModelData>().gfxMeshIndices.front()*/;
	}
	
	void MeshRendererComponent::SetModelHandle(Asset _asset, uint32_t _submodel_id)
	{
		submodel_id = _submodel_id;
		mesh_handle = _asset;

		model_handle = mesh_handle.GetData<ModelData*>()->gfxMeshIndices[submodel_id];
	}
	
	Asset MeshRendererComponent::GetMesh()
	{
		return mesh_handle;
	}
	
	void MeshRendererComponent::SetMesh(Asset _asset)
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
	
	int MeshRendererComponent::GetSubModelID()
	{
		return submodel_id;
	}
	
	void MeshRendererComponent::SetSubModelID(int id)
	{
		if (mesh_handle.IsValid())
		{
			auto modelData = mesh_handle.GetData<ModelData*>();
			id = id % modelData->gfxMeshIndices.size();
			submodel_id = id % modelData->gfxMeshIndices.size();
			model_handle = modelData->gfxMeshIndices[submodel_id];
		}
	}
}
