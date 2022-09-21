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

namespace oo
{
    Asset::Asset(std::filesystem::path contentPath, AssetID id)
        : id{ id }
        , info{ id == ID_NULL ? new AssetInfo() : nullptr }
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
        : info{ other.info }
    {
        if (info)
        {
            info->copies.emplace_back(this);
        }
    }

    Asset::Asset(Asset&& other)
        : info{ other.info }
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

    inline std::vector<std::type_index> Asset::GetBespokeTypes() const
    {
        auto v = std::vector<std::type_index>();
        for (auto it = info->dataTypeOffsets.begin(); it != info->dataTypeOffsets.end(); ++it)
        {
            v.emplace_back(it->first);
        }
        return v;
    }

    void Asset::createData()
    {
        info->onAssetCreate(*info);
    }

    void Asset::destroyData()
    {
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
