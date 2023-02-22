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
        MeshInfo MeshInformation;
    
        Asset AlbedoHandle;
        uint32_t AlbedoID = 0xFFFFFFFF;
        
        Asset NormalHandle;
        uint32_t NormalID = 0xFFFFFFFF;
        
        Asset MetallicHandle;
        uint32_t MetallicID = 0xFFFFFFFF;
        
        Asset RoughnessHandle;
        uint32_t RoughnessID = 0xFFFFFFFF;

        //no need to serialize
        uint32_t ModelHandle{ 0 };
        int32_t GraphicsWorldID{};
        int32_t PickingID;

        bool CastShadows = false;
        bool ReceiveShadows = false;

        MeshInfo GetMeshInfo();
        /*********************************************************************************//*!
        \brief      this function will only set the submeshbits
        *//**********************************************************************************/
        void SetMeshInfo(MeshInfo info);
        
        Asset GetMesh();
        void SetMesh(Asset _asset);

        //void GetModelHandle();

        //set a single model and asset
        void SetModelHandle(Asset _asset, uint32_t _submodel_id);
        
        void SetAlbedoMap(Asset albedoMap);
        Asset GetAlbedoMap() const;

        void SetNormalMap(Asset normalMap);
        Asset GetNormalMap() const;

        void SetMetallicMap(Asset metallicMap);
        Asset GetMetallicMap() const;

        void SetRoughnessMap(Asset roughnessMap);
        Asset GetRoughnessMap() const;

        RTTR_ENABLE();
    };

}