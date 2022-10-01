/************************************************************************************//*!
\file           RendererComponent.h
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

#pragma once
#include "Ouroboros/Asset/AssetManager.h"

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

		void GetModelHandle();
		void SetModelHandle(Asset _asset, uint32_t _submodel_id);
		Asset GetMesh();
		void SetMesh(Asset _asset);
		int GetSubModelID();
		void SetSubModelID(int id);

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
