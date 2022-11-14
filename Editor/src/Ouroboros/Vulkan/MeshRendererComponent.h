/************************************************************************************//*!
\file           MeshRendererComponent.h
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

#include "glm/common.hpp"
#include "Ouroboros/Asset/AssetManager.h"
#include "MeshInfo.h"
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
        uint32_t model_handle{ 0 };
        uint32_t graphicsWorld_ID{};

        bool CastShadows = false;
        bool ReceiveShadows = false;

        MeshInfo GetMeshInfo();
        /*********************************************************************************//*!
        \brief      this function will only set the submeshbits
        *//**********************************************************************************/
        void SetMeshInfo(MeshInfo info);
        void GetModelHandle();

        //set a single model and asset
        void SetModelHandle(Asset _asset, uint32_t _submodel_id);
        Asset GetMesh();
        void SetMesh(Asset _asset);

        void SetAlbedoMap(Asset albedoMap);
        Asset GetAlbedoMap() const;

        void SetNormalMap(Asset normalMap);
        Asset GetNormalMap() const;

        //std::vector<Material> materials;

        //Lighting

        //Probes

        //Additional Settings
        RTTR_ENABLE();
    };

}