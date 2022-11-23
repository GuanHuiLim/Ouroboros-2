/************************************************************************************//*!
\file           MeshRendererComponent.cpp
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
#include "MeshRendererComponent.h"
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
        .property_readonly("Graphics World ID", &MeshRendererComponent::GraphicsWorldID)
        .property_readonly("Model Handle", &MeshRendererComponent::ModelHandle)
        .property("Albedo", &MeshRendererComponent::GetAlbedoMap, &MeshRendererComponent::SetAlbedoMap)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
        )
        .property("Normal", &MeshRendererComponent::GetNormalMap, &MeshRendererComponent::SetNormalMap)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
        )
        .property("Metallic", &MeshRendererComponent::GetMetallicMap, &MeshRendererComponent::SetMetallicMap)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
        )
        .property("Roughness", &MeshRendererComponent::GetRoughnessMap, &MeshRendererComponent::SetRoughnessMap)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
        )
        .property("Mesh", &MeshRendererComponent::GetMesh, &MeshRendererComponent::SetMesh)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Model))
        )
        .property("MeshInfo", &MeshRendererComponent::GetMeshInfo, &MeshRendererComponent::SetMeshInfo)
        .property("Cast Shadows", &MeshRendererComponent::CastShadows)
        .property("Receive Shadows", &MeshRendererComponent::ReceiveShadows)
            ;
    }

    MeshInfo MeshRendererComponent::GetMeshInfo()
    {
        return MeshInformation;
    }

    /*********************************************************************************//*!
    \brief      this function will only set the submeshbits
    *//**********************************************************************************/
    void MeshRendererComponent::SetMeshInfo(MeshInfo info)
    {
        MeshInformation.submeshBits = info.submeshBits;
    }

    //void MeshRendererComponent::GetModelHandle()
    //{
    //    if (MeshInformation.mesh_handle.HasData())
    //        ModelHandle = MeshInformation.mesh_handle.GetData<ModelFileResource*>()->meshResource;
    //    else
    //        ModelHandle = 0;

    //    //ModelHandle /*= MeshInformation.mesh_handle.GetData<ModelFileResource>().meshResource*/;
    //}

    //set a single model and asset

    void MeshRendererComponent::SetModelHandle(Asset _asset, uint32_t _submodel_id)
    {
        MeshInformation.submeshBits.reset();
        MeshInformation.submeshBits[_submodel_id] = true;
        MeshInformation.mesh_handle = _asset;

        ModelHandle = MeshInformation.mesh_handle.GetData<ModelFileResource*>()->meshResource;
    }

    Asset MeshRendererComponent::GetMesh()
    {
        return MeshInformation.mesh_handle;
    }

    void MeshRendererComponent::SetMesh(Asset _asset)
    {
        MeshInformation.mesh_handle = _asset;

        if (_asset.IsValid())
        {
            ModelHandle = MeshInformation.mesh_handle.GetData<ModelFileResource*>()->meshResource;
            // HACK this is needed to render stuff under edit..
            // MeshInformation.submeshBits.reset();
            // MeshInformation.submeshBits[0] = true;
        }
        else
        {
            ModelHandle = 0;
        }
    }

    void oo::MeshRendererComponent::SetAlbedoMap(Asset albedoMap)
    {
        AlbedoHandle = albedoMap;
        if (AlbedoHandle.IsValid())
        {
            AlbedoID = AlbedoHandle.GetData<uint32_t>();
        }
        else
        {
            AlbedoID = 0xFFFFFFFF;
        }
    }

    Asset oo::MeshRendererComponent::GetAlbedoMap() const
    {
        return AlbedoHandle;
    }

    void oo::MeshRendererComponent::SetNormalMap(Asset normalMap)
    {
        NormalHandle = normalMap;
        if (NormalHandle.IsValid())
        {
            NormalID = NormalHandle.GetData<uint32_t>();
        }
        else
        {
            NormalID = 0xFFFFFFFF;
        }
    }

    Asset oo::MeshRendererComponent::GetNormalMap() const
    {
        return NormalHandle;
    }

    void oo::MeshRendererComponent::SetMetallicMap(Asset metallicMap)
    {
        MetallicHandle = metallicMap;
        if (MetallicHandle.IsValid())
        {
            MetallicID = MetallicHandle.GetData<uint32_t>();
        }
        else
        {
            MetallicID = 0xFFFFFFFF;
        }
    }

    Asset oo::MeshRendererComponent::GetMetallicMap() const
    {
        return MetallicHandle;
    }

    void oo::MeshRendererComponent::SetRoughnessMap(Asset roughnessMap)
    {
        RoughnessHandle = roughnessMap;
        if (RoughnessHandle.IsValid())
        {
            RoughnessID = RoughnessHandle.GetData<uint32_t>();
        }
        else
        {
            RoughnessID = 0xFFFFFFFF;
        }
    }

    Asset oo::MeshRendererComponent::GetRoughnessMap() const
    {
        return RoughnessHandle;
    }

}
