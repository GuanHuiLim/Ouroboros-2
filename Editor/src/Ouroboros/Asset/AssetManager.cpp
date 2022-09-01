/************************************************************************************//*!
\file           AssetManager.cpp
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Aug 30, 2022
\brief          Contains the definition for the AssetManager class.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

#include "AssetManager.h"

#include "BinaryIO.h"
#include "Ouroboros/Vulkan/VulkanContext.h"

namespace oo
{
    AssetManager::AssetManager(std::filesystem::path root)
        : root{ root }
    {
        fileWatchThread = std::thread(&AssetManager::fileWatch, this);
    }

    AssetManager::~AssetManager()
    {
        isRunning = false;
        fileWatchThread.join();
    }

    Asset AssetManager::Get(const AssetID& snowflake)
    {
        // Get asset from asset store
        if (assets.contains(snowflake))
        {
            const auto FP = assets[snowflake].GetFilePath();
            if (std::filesystem::exists(FP))
            {
                return assets.at(snowflake);
            }

            // Remove meta file
            auto fpMeta = FP;
            fpMeta += Asset::EXT_META;
            if (std::filesystem::exists(fpMeta))
            {
                std::filesystem::remove(fpMeta);
            }

            assets.erase(snowflake);
        }
        throw AssetNotFoundException();
    }

    std::future<Asset> AssetManager::GetAsync(const AssetID& snowflake)
    {
        return std::async(std::launch::async, &AssetManager::Get, this, snowflake);
    }

    Asset AssetManager::LoadPath(const std::filesystem::path& fp)
    {
        const auto FP = root / fp;

        if (!std::filesystem::exists(FP))
            throw AssetNotFoundException();

        // Get file paths
        auto fpContent = FP;
        auto fpMeta = fpContent;
        if (fpContent.extension() == Asset::EXT_META)
        {
            fpContent.replace_extension();
        }
        else
        {
            fpMeta += Asset::EXT_META;
        }

        // Ensure meta file exists
        AssetMetaContent meta;
        if (!std::filesystem::exists(fpMeta))
        {
            meta.id = Asset::GenerateSnowflake();
            std::ofstream ofs = std::ofstream(fpMeta);
            BinaryIO::Write(ofs, meta);
        }
        else
        {
            std::ifstream ifs = std::ifstream(fpMeta);
            BinaryIO::Read(ifs, meta);
        }

        // Get or load asset
        if (assets.contains(meta.id))
        {
            // Get asset
            return assets.at(meta.id);
        }
        else
        {
            // Load asset
            // TODO: store different data depending on asset type
            //asset.meta->data = new std::ifstream(fpAsset);
            Asset asset = Asset(fpContent);
            assets.insert({ meta.id, asset });
            return asset;
        }
    }

    std::future<Asset> AssetManager::LoadPathAsync(const std::filesystem::path& fp)
    {
        return std::async(std::launch::async, &AssetManager::LoadPath, this, fp);
    }

    void AssetManager::fileWatch()
    {
        const std::filesystem::path DIR = std::filesystem::canonical(root);

        std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
        while (isRunning)
        {
            // Check for creation or modification
            for (auto& file : std::filesystem::recursive_directory_iterator(DIR))
            {
                const std::filesystem::path FP = std::filesystem::canonical(file.path());
                const std::filesystem::path FP_EXT = FP.extension();

                if (FP_EXT == Asset::EXT_META)
                    continue;

                auto fpMeta = FP;
                fpMeta += Asset::EXT_META;

                const auto WRITE_TIME = std::filesystem::last_write_time(file.path());
                if (t.time_since_epoch() < WRITE_TIME.time_since_epoch())
                {
                    // Updated
                    AssetMetaContent meta;
                    std::ifstream ifs = std::ifstream(fpMeta);
                    BinaryIO::Read(ifs, meta);
                    if (!assets.contains(meta.id))
                    {
                        // Created
                        LoadPath(FP);
                    }
                }
            }

            // Check time elapsed
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t).count() < WATCH_INTERVAL)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            t = now;
        }
    }
}
