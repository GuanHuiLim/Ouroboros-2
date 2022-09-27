/************************************************************************************//*!
\file           Asset.cpp
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Aug 30, 2022
\brief          Contains the definition for the Asset and AssetHeader classes.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

#include "Asset.h"

#include "OO_Vulkan/src/MeshModel.h"
#include "Ouroboros/Audio/Audio.h"
#include "Ouroboros/Core/Application.h"
#include "Ouroboros/Vulkan/VulkanContext.h"
#include "Utility/IEqual.h"

namespace
{

    template<typename T, typename It>
    inline bool findIn(const T& obj, const It& itBegin, const It& itEnd)
    {
        return std::find_if(itBegin, itEnd, [obj](const auto& e)
        {
            return iequal(e, obj);
        }) != itEnd;
    }
}

namespace oo
{
    Asset::Asset(std::filesystem::path contentPath, AssetID id)
        : id{ id }
        , info{ id != ID_NULL ? new AssetInfo() : nullptr }
    {
        if (info)
        {
            info->copies.emplace_back(this);
            info->contentPath = contentPath;
            info->metaPath = contentPath;
            info->metaPath += EXT_META;
        }
    }

    Asset::Asset(const Asset& other)
        : id{ other.id }
        , info{ other.info }
    {
        if (info)
        {
            info->copies.emplace_back(this);
        }
    }

    Asset::Asset(Asset&& other)
        : id{ other.id }
        , info{ other.info }
    {
        if (info)
        {
            info->copies.remove(&other);
            info->copies.emplace_back(this);
        }
        other.info = nullptr;
    }

    Asset& Asset::operator=(const Asset& other)
    {
        this->Asset::~Asset();
        this->Asset::Asset(other);
        return *this;
    }

    Asset& Asset::operator=(Asset&& other)
    {
        this->Asset::~Asset();
        this->Asset::Asset(std::forward<Asset>(other));
        return *this;
    }

    Asset::~Asset()
    {
        if (info)
        {
            info->copies.remove(this);
            if (info->copies.empty())
            {
                // Free
                info->onAssetDestroy(*info);
                if (info->data)
                    delete info->data;
                delete info;
            }
        }
        info = nullptr;
    }

    void Asset::Reload()
    {
        const auto FP_EXT = info->contentPath.extension();
        if (findIn(FP_EXT.string(), Asset::EXTS_TEXTURE.begin(), Asset::EXTS_TEXTURE.end()))
        {
            Reload(AssetInfo::Type::Texture);
        }
        else if (findIn(FP_EXT.string(), Asset::EXTS_AUDIO.begin(), Asset::EXTS_AUDIO.end()))
        {
            Reload(AssetInfo::Type::Audio);
        }
        else if (findIn(FP_EXT.string(), Asset::EXTS_MODEL.begin(), Asset::EXTS_MODEL.end()))
        {
            Reload(AssetInfo::Type::Model);
        }
    }

    void Asset::Reload(AssetInfo::Type type)
    {
        // Call old asset destruction callback
        destroyData();

        switch (type)
        {
            case AssetInfo::Type::Texture:
            {
                // Load texture
                info->type = AssetInfo::Type::Texture;
                info->onAssetCreate = [](AssetInfo& self)
                {
                    auto vc = Application::Get().GetWindow().GetVulkanContext();
                    auto vr = vc->getRenderer();
                    auto data1 = vr->CreateTexture(self.contentPath.string());
                    auto data2 = vr->GetImguiID(data1);

                    struct DataStruct
                    {
                        decltype(data1) data1;
                        decltype(data2) data2;
                    };
                    self.data = new DataStruct;
                    *reinterpret_cast<DataStruct*>(self.data) = {
                        .data1 = data1,
                        .data2 = data2
                    };
                    self.dataTypeOffsets = {};
                    self.dataTypeOffsets[std::type_index(typeid(decltype(data1)))] = offsetof(DataStruct, data1);
                    self.dataTypeOffsets[std::type_index(typeid(decltype(data2)))] = offsetof(DataStruct, data2);
                };
                info->onAssetDestroy = [this](AssetInfo& self)
                {
                    // TODO: Unload texture
                    if (self.data)
                        delete self.data;
                    self.data = nullptr;
                    self.dataTypeOffsets.clear();
                };
                break;
            }
            case AssetInfo::Type::Audio:
            {
                // Load audio
                info->type = AssetInfo::Type::Audio;
                info->onAssetCreate = [](AssetInfo& self)
                {
                    auto data1 = audio::CreateSound(self.contentPath.string());

                    struct DataStruct
                    {
                        decltype(data1) data1;
                    };
                    self.data = new DataStruct;
                    *reinterpret_cast<DataStruct*>(self.data) = {
                        .data1 = data1,
                    };
                    self.dataTypeOffsets = {};
                    self.dataTypeOffsets[std::type_index(typeid(decltype(data1)))] = offsetof(DataStruct, data1);
                };
                info->onAssetDestroy = [](AssetInfo& self)
                {
                    if (self.data)
                    {
                        audio::FreeSound(self.GetData<oo::SoundID>());
                        delete self.data;
                    }
                    self.data = nullptr;
                    self.dataTypeOffsets.clear();
                };
                break;
            }
            case AssetInfo::Type::Model:
            {
                // Load model
                info->type = AssetInfo::Type::Model;
                info->onAssetCreate = [](AssetInfo& self)
                {
                    auto vc = Application::Get().GetWindow().GetVulkanContext();
                    auto vr = vc->getRenderer();
                    auto data1 = vr->LoadModelFromFile(self.contentPath.string());

                    struct DataStruct
                    {
                        decltype(data1) data1;
                    };
                    self.data = new DataStruct;
                    *reinterpret_cast<DataStruct*>(self.data) = {
                        .data1 = data1,
                    };
                    self.dataTypeOffsets = {};
                    self.dataTypeOffsets[std::type_index(typeid(decltype(data1)))] = offsetof(DataStruct, data1);
                };
                info->onAssetDestroy = [](AssetInfo& self)
                {
                    if (self.data)
                    {
                        delete self.GetData<ModelData*>();
                        delete self.data;
                    }
                    self.data = nullptr;
                    self.dataTypeOffsets.clear();
                };
                break;
            }
        }

        // Call asset creation callback
        createData();
    }

    void Asset::Overwrite()
    {
        // TODO: impl
    }

    std::vector<std::type_index> Asset::GetBespokeTypes() const
    {
        auto v = std::vector<std::type_index>();
        for (auto it = info->dataTypeOffsets.begin(); it != info->dataTypeOffsets.end(); ++it)
        {
            v.emplace_back(it->first);
        }
        return v;
    }

    gfxModel& Asset::GetSubmodel(size_t index)
    {
        if (info->type != AssetInfo::Type::Model)
            throw AssetInvalidTypeException();

        auto data = GetData<ModelData*>();
        auto vc = Application::Get().GetWindow().GetVulkanContext();
        auto vr = vc->getRenderer();
        return vr->models[data->gfxMeshIndices[index]];
    }

    size_t Asset::GetSubmodelCount() const
    {
        if (info->type != AssetInfo::Type::Model)
            throw AssetInvalidTypeException();

        auto data = GetData<ModelData*>();
        return data->gfxMeshIndices.size();
    }

    void Asset::createData()
    {
        if (info && info->onAssetCreate)
            info->onAssetCreate(*info);
    }

    void Asset::destroyData()
    {
        if (info && info->onAssetDestroy)
            info->onAssetDestroy(*info);
    }

    AssetID Asset::GenerateSnowflake()
    {
        // Internal sequence number up to 65535
        static uint16_t sequence = 0;

        // Time at ms precision
        auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
        auto time = now.time_since_epoch();

        // 48 bits of time, 16 bits of sequence number
        return (time.count() << 16) | ++sequence;
    }
}
