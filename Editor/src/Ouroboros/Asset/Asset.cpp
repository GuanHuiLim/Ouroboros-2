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

    oo::Snowflake GenerateSnowflake()
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

namespace oo
{
    AssetInfo::~AssetInfo()
    {
        if (onAssetDestroy)
            onAssetDestroy(*this);
    }

    Snowflake AssetInfo::GenerateSnowflake()
    {
        return ::GenerateSnowflake();
    }

    void AssetInfo::Reload()
    {
        const auto FP_EXT = contentPath.extension();
        if (findIn(FP_EXT.string(), Asset::EXTS_TEXTURE.begin(), Asset::EXTS_TEXTURE.end()))
            Reload(AssetInfo::Type::Texture);
        else if (findIn(FP_EXT.string(), Asset::EXTS_AUDIO.begin(), Asset::EXTS_AUDIO.end()))
            Reload(AssetInfo::Type::Audio);
        else if (findIn(FP_EXT.string(), Asset::EXTS_MODEL.begin(), Asset::EXTS_MODEL.end()))
            Reload(AssetInfo::Type::Model);
    }

    void AssetInfo::Reload(AssetInfo::Type t)
    {
        // Call old asset destruction callback
        if (onAssetDestroy)
            onAssetDestroy(*this);

        type = t;
        switch (type)
        {
            case AssetInfo::Type::Texture:
            {
                // Load texture
                onAssetCreate = [](AssetInfo& self)
                {
                    auto vc = Application::Get().GetWindow().GetVulkanContext();
                    auto vr = vc->getRenderer();
                    auto tid = vr->CreateTexture(self.contentPath.string());
                    self.data.emplace_back(tid);
                    self.data.emplace_back(vr->GetImguiID(tid));
                };
                onAssetDestroy = [this](AssetInfo& self)
                {
                    // TODO: Unload texture
                    //const auto& value = self.GetData<AssetInfo::TextureData>();
                };
                break;
            }
            case AssetInfo::Type::Audio:
            {
                // Load audio
                onAssetCreate = [](AssetInfo& self)
                {
                    self.data.emplace_back(audio::CreateSound(self.contentPath.string()));
                };
                onAssetDestroy = [](AssetInfo& self)
                {
                    audio::FreeSound(self.GetData<oo::SoundID>());
                };
                break;
            }
            case AssetInfo::Type::Model:
            {
                // Load model
                onAssetCreate = [](AssetInfo& self)
                {
                    auto vc = Application::Get().GetWindow().GetVulkanContext();
                    auto vr = vc->getRenderer();
                    self.data.emplace_back(std::shared_ptr<ModelFileResource>(vr->LoadModelFromFile(self.contentPath.string())));
                };
                onAssetDestroy = [](AssetInfo& self)
                {
                    //delete self.GetData<ModelFileResource*>();
                };
                break;
            }
        }

        // Call asset creation callback
        if (onAssetCreate)
            onAssetCreate(*this);
    }

    void AssetInfo::Overwrite()
    {
        // TODO
    }

    Snowflake Asset::GenerateSnowflake()
    {
        return ::GenerateSnowflake();
    }

    void Asset::Reload()
    {
        if (!IsValid())
            return;

        if (auto sp = info.lock())
            sp->Reload();
    }

    void Asset::Reload(AssetInfo::Type type)
    {
        if (!IsValid())
            return;

        if (auto sp = info.lock())
            sp->Reload(type);
    }

    void Asset::Overwrite()
    {
        if (!IsValid())
            return;

        if (auto sp = info.lock())
            sp->Overwrite();
    }

    size_t Asset::GetSubmodelCount() const
    {
        if (GetType() != AssetInfo::Type::Model)
        {
            throw AssetInvalidTypeException();
        }
        auto data = GetData<ModelFileResource*>();
        return data->numSubmesh;
    }
}
