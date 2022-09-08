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
#include "Ouroboros/Core/Application.h"
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

        return getOrLoadAbsolute(FP);
    }

    std::future<Asset> AssetManager::LoadPathAsync(const std::filesystem::path& fp)
    {
        return std::async(std::launch::async, &AssetManager::LoadPath, this, fp);
    }

    std::vector<Asset> AssetManager::LoadName(const std::filesystem::path& fn)
    {
        const std::filesystem::path DIR = std::filesystem::canonical(root);

        std::vector<Asset> v;
        for (auto& file : std::filesystem::recursive_directory_iterator(DIR))
        {
            if (file.path().filename() == fn)
            {
                v.emplace_back(getOrLoadAbsolute(file.path()));
            }
        }
        return v;
    }

    std::future<std::vector<Asset>> AssetManager::LoadNameAsync(const std::filesystem::path& fn)
    {
        return std::async(std::launch::async, &AssetManager::LoadName, this, fn);
    }

    void AssetManager::fileWatch()
    {
        std::chrono::file_clock::time_point tLast = std::chrono::file_clock::now();
        std::chrono::file_clock::time_point t = std::chrono::file_clock::now();
        while (isRunning)
        {
            const std::filesystem::path DIR = std::filesystem::canonical(root);
            if (std::filesystem::exists(DIR))
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
                    if (tLast < WRITE_TIME && WRITE_TIME <= t)
                    {
                        // Updated
                        AssetMetaContent meta;
                        std::ifstream ifs = std::ifstream(fpMeta);
                        BinaryIO::Read(ifs, meta);
                        if (!assets.contains(meta.id))
                        {
                            // Created
                            LoadPath(FP);
                            std::cout << "Created " << FP << "\n";
                        }
                        else
                        {
                            // Modified
                            assets[meta.id].info->timeLoaded = t;
                            assets[meta.id].destroyData();
                            assets[meta.id].createData();
                            std::cout << "Modified " << FP << "\n";
                        }
                    }
                }
            }

            // Check time elapsed
            std::chrono::file_clock::time_point now = std::chrono::file_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t).count() < WATCH_INTERVAL)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            tLast = t;
            t = now;
        }
    }

    Asset AssetManager::getOrLoadAbsolute(const std::filesystem::path& fp)
    {
        // Get file paths
        auto fpContent = fp;
        auto fpMeta = fpContent;
        if (fpContent.extension() == Asset::EXT_META)
        {
            fpContent.replace_extension();
        }
        else
        {
            fpMeta += Asset::EXT_META;
        }
        const auto FP_EXT = fpContent.extension();

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
            Asset asset = createAsset(fpContent);
            assets.insert({ meta.id, asset });
            return asset;
        }
    }

    Asset AssetManager::createAsset(std::filesystem::path fp)
    {
        const auto FP_EXT = fp.extension();
        Asset asset = Asset(fp);
        if (std::find(Asset::EXTS_TEXTURE.begin(), Asset::EXTS_TEXTURE.end(), FP_EXT) != Asset::EXTS_TEXTURE.end())
        {
            // Load texture
            asset.info->onAssetCreate = [fp](AssetInfo& self)
            {
            	auto vc = Application::Get().GetWindow().GetVulkanContext();
                auto vr = vc->getRenderer();
                auto data = vr->CreateTexture(fp.string());
                self.data = new decltype(data);
                *reinterpret_cast<decltype(data)*>(self.data) = data;
            };
            asset.info->onAssetDestroy = [fp](AssetInfo& self)
            {
                // TODO: Unload texture
                if (self.data)
                    delete self.data;
                self.data = nullptr;
            };
        }

        // Call asset creation callback
        asset.createData();

        return asset;
    }
}
