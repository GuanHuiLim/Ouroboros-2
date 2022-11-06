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
#include "Ouroboros/Animation/Animation.h"
#include "Ouroboros/Animation/AnimationSystem.h"
#include "Ouroboros/Animation/AnimationTree.h"
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
        Unload();
    }

    Snowflake AssetInfo::GenerateSnowflake()
    {
        return ::GenerateSnowflake();
    }

#define REGISTER_TYPE(_EXT_LIST, _TYPE)                                 \
    if (findIn(FP_EXT.string(), _EXT_LIST.begin(), _EXT_LIST.end()))    \
        return AssetInfo::Type::_TYPE;

    AssetInfo::Type AssetInfo::GetType() const
    {
        const auto FP_EXT = contentPath.extension();
        REGISTER_TYPE(Asset::EXTS_TEXTURE, Texture);
        REGISTER_TYPE(Asset::EXTS_AUDIO, Audio);
        REGISTER_TYPE(Asset::EXTS_MODEL, Model);
        REGISTER_TYPE(Asset::EXTS_ANIMATION, Animation);
        REGISTER_TYPE(Asset::EXTS_ANIMATION_TREE, AnimationTree);
        return AssetInfo::Type::Text;
    }

#undef REGISTER_TYPE

    void AssetInfo::Reload()
    {
        Reload(GetType());
    }

    void AssetInfo::Reload(AssetInfo::Type t)
    {
        Unload();

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
                    auto sp = std::shared_ptr<ModelFileResource>(vr->LoadModelFromFile(self.contentPath.string()));
                    self.data.emplace_back(sp);

                    auto anims = Anim::Animation::LoadAnimationFromFBX(self.contentPath.string(), sp.get());
                    auto v = std::vector<std::string>();
                    for (auto& anim : anims)
                    {
                        v.emplace_back(anim->name);
                    }
                    self.data.emplace_back(v);
                };
                onAssetDestroy = [](AssetInfo& self) {};
                break;
            }
            case AssetInfo::Type::Animation:
            {
                // Load animation
                onAssetCreate = [](AssetInfo& self)
                {
                    using namespace Anim;
                    Animation* anim = AnimationSystem::LoadAnimation(self.contentPath.string());
                    self.data.emplace_back(anim->name);
                };
                onAssetDestroy = [](AssetInfo& self) {};
                break;
            }
            case AssetInfo::Type::AnimationTree:
            {
                // Load animation tree
                onAssetCreate = [](AssetInfo& self)
                {
                    using namespace Anim;
                    AnimationTree* animTree = AnimationSystem::LoadAnimationTree(self.contentPath.string());
                    self.data.emplace_back(animTree->name);
                };
                onAssetDestroy = [](AssetInfo& self) {};
                break;
            }
        }

        // Call asset creation callback
        if (onAssetCreate)
            onAssetCreate(*this);

        // Mark as data loaded
        isDataLoaded = true;
        timeLoaded = std::chrono::file_clock::now();
    }

    void AssetInfo::Unload()
    {
        if (!isDataLoaded)
            return;

        // Call old asset destruction callback
        if (onAssetDestroy)
            onAssetDestroy(*this);

        // Clear data
        data.clear();

        // Mark as data unloaded
        isDataLoaded = false;
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

    void Asset::Unload()
    {
        if (!IsValid())
            return;

        if (auto sp = info.lock())
            sp->Unload();
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
