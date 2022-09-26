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

#include <imgui/imgui.h>

#include "BinaryIO.h"
#include "Ouroboros/Core/Application.h"
#include "Ouroboros/Vulkan/VulkanContext.h"

namespace oo
{
    Asset& AssetManager::AssetStore::At(AssetID id)
    {
        return assets.at(id);
    }

    Asset AssetManager::AssetStore::At(AssetID id) const
    {
        return assets.at(id);
    }

    std::vector<std::reference_wrapper<Asset>> AssetManager::AssetStore::At(AssetInfo::Type type)
    {
        std::vector<std::reference_wrapper<Asset>> v;
        if (assetsByType.contains(type))
        {
            auto& vid = assetsByType.at(type);
            std::transform(vid.begin(), vid.end(), std::back_inserter(v), [this](const AssetID& e)
            {
                return std::ref(assets.at(e));
            });
        }
        return v;
    }

    std::vector<Asset> AssetManager::AssetStore::At(AssetInfo::Type type) const
    {
        std::vector<Asset> v;
        if (assetsByType.contains(type))
        {
            auto& vid = assetsByType.at(type);
            std::transform(vid.begin(), vid.end(), std::back_inserter(v), [this](const AssetID& e)
            {
                return assets.at(e);
            });
        }
        return v;
    }

    Asset AssetManager::AssetStore::Insert(AssetID id, const Asset& asset)
    {
        assets.insert({ id, asset });
        if (!assetsByType.contains(asset.GetType()))
            assetsByType.insert({ asset.GetType(), {} });
        assetsByType.at(asset.GetType()).insert(id);
        return asset;
    }

    void AssetManager::AssetStore::Erase(AssetID id)
    {
        if (assets.contains(id))
        {
            const auto& ASSET = assets.at(id);
            assetsByType.at(ASSET.GetType()).erase(id);
            assets.erase(id);
        }
    }

    bool AssetManager::AssetStore::Contains(AssetID id) const
    {
        return assets.contains(id);
    }

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
        if (assets.Contains(snowflake))
        {
            const auto& ASSET = assets.At(snowflake);
            const auto FP = ASSET.GetFilePath();
            if (std::filesystem::exists(FP))
            {
                return ASSET;
            }

            // Remove meta file
            auto fpMeta = FP;
            fpMeta += Asset::EXT_META;
            if (std::filesystem::exists(fpMeta))
            {
                std::filesystem::remove(fpMeta);
            }

            assets.Erase(snowflake);
        }
        return Asset();
    }

    std::future<Asset> AssetManager::GetAsync(const AssetID& snowflake)
    {
        return std::async(std::launch::async, &AssetManager::Get, this, snowflake);
    }

    std::vector<Asset> AssetManager::GetLoadedAssetsByType(AssetInfo::Type type) const
    {
        return assets.At(type);
    }

    Asset AssetManager::LoadPath(const std::filesystem::path& fp)
    {
        const auto FP = root / fp;

        if (!std::filesystem::exists(FP))
            return Asset();

        return getOrLoadAbsolute(FP);
    }

    std::future<Asset> AssetManager::LoadPathAsync(const std::filesystem::path& fp)
    {
        return std::async(std::launch::async, &AssetManager::LoadPath, this, fp);
    }

    std::vector<Asset> AssetManager::LoadDirectory(const std::filesystem::path& path, bool recursive)
    {
        const auto PATH = root / path;

        if (!std::filesystem::exists(PATH) || !std::filesystem::is_directory(PATH))
            return {};

        std::vector<Asset> v;
        if (recursive)
        {
            for (auto& file : std::filesystem::recursive_directory_iterator(PATH))
            {
                v.emplace_back(getOrLoadAbsolute(file.path()));
            }
        }
        else
        {
            for (auto& file : std::filesystem::directory_iterator(PATH))
            {
                v.emplace_back(getOrLoadAbsolute(file.path()));
            }
        }
        return v;
    }

    std::future<std::vector<Asset>> AssetManager::LoadDirectoryAsync(const std::filesystem::path& path, bool recursive)
    {
        return std::async(std::launch::async, &AssetManager::LoadDirectory, this, path, recursive);
    }

    std::vector<Asset> AssetManager::LoadName(const std::filesystem::path& fn, bool caseSensitive)
    {
        const std::filesystem::path DIR = std::filesystem::canonical(root);

        std::vector<Asset> v;
        for (auto& file : std::filesystem::recursive_directory_iterator(DIR))
        {
            if ((caseSensitive && file.path().filename() == fn) ||
                (!caseSensitive && iequal(file.path().filename().string(), fn.string())))
            {
                v.emplace_back(getOrLoadAbsolute(file.path()));
            }
        }
        return v;
    }

    std::future<std::vector<Asset>> AssetManager::LoadNameAsync(const std::filesystem::path& fn, bool caseSensitive)
    {
        return std::async(std::launch::async, &AssetManager::LoadName, this, fn, caseSensitive);
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
                // Check root
                const auto ROOT_WRITE_TIME = std::filesystem::last_write_time(DIR);
                if (tLast < ROOT_WRITE_TIME && ROOT_WRITE_TIME <= t)
                {
                    updateAssetPaths(DIR);
                }

                // Iterate root
                for (auto& fp : std::filesystem::recursive_directory_iterator(DIR))
                {
                    const std::filesystem::path FP = std::filesystem::canonical(fp.path());
                    if (!std::filesystem::is_regular_file(FP))
                        continue;

                    const std::filesystem::path FP_EXT = FP.extension();
                    if (FP_EXT == Asset::EXT_META)
                        continue;

                    const auto WRITE_TIME = std::filesystem::last_write_time(fp.path());
                    if (WRITE_TIME <= tLast || t < WRITE_TIME)
                        continue;

                    auto fpMeta = FP;
                    fpMeta += Asset::EXT_META;

                    AssetMetaContent meta;
                    std::ifstream ifs = std::ifstream(fpMeta);
                    BinaryIO::Read(ifs, meta);
                    if (!assets.Contains(meta.id))
                    {
                        // Created
                        LoadPath(FP);
                        std::cout << "Created " << FP << "\n";
                    }
                    else
                    {
                        // Modified
                        assets.At(meta.id).info->contentPath = FP;
                        assets.At(meta.id).info->metaPath = fpMeta;
                        assets.At(meta.id).info->timeLoaded = t;
                        assets.At(meta.id).destroyData();
                        assets.At(meta.id).createData();
                        std::cout << "Modified " << FP << "\n";
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

    void AssetManager::updateAssetPaths(const std::filesystem::path& dir)
    {
        for (auto& fp : std::filesystem::directory_iterator(dir))
        {
            if (std::filesystem::is_regular_file(fp))
            {
                const std::filesystem::path FP = std::filesystem::canonical(fp.path());

                const std::filesystem::path FP_EXT = FP.extension();
                if (FP_EXT == Asset::EXT_META)
                    continue;

                auto fpMeta = FP;
                fpMeta += Asset::EXT_META;
                if (!std::filesystem::exists(fpMeta))
                    continue;

                AssetMetaContent meta;
                std::ifstream ifs = std::ifstream(fpMeta);
                BinaryIO::Read(ifs, meta);
                if (assets.Contains(meta.id) && assets.At(meta.id).info->contentPath != FP)
                {
                    // Moved
                    assets.At(meta.id).info->contentPath = FP;
                    assets.At(meta.id).info->metaPath = fpMeta;
                    std::cout << "Moved " << FP << "\n";
                }
            }
            else if (std::filesystem::is_directory(fp))
            {
                updateAssetPaths(fp);
            }
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
        if (assets.Contains(meta.id))
        {
            // Get asset
            return assets.At(meta.id);
        }
        else
        {
            // Load asset
            Asset asset = createAsset(fpContent, meta.id);
            assets.Insert(asset.id, asset);
            return asset;
        }
    }

    Asset AssetManager::createAsset(std::filesystem::path fp, AssetID id)
    {
        const auto FP_EXT = fp.extension();
        Asset asset = Asset(std::filesystem::canonical(fp), id);
        if (findIn(FP_EXT.string(), Asset::EXTS_TEXTURE.begin(), Asset::EXTS_TEXTURE.end()))
        {
            // Load texture
            asset.info->type = AssetInfo::Type::Texture;
            asset.info->onAssetCreate = [fp](AssetInfo& self)
            {
                auto vc = Application::Get().GetWindow().GetVulkanContext();
                auto vr = vc->getRenderer();
                auto data1 = vr->CreateTexture(fp.string());
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
            asset.info->onAssetDestroy = [fp](AssetInfo& self)
            {
                // TODO: Unload texture
                if (self.data)
                    delete self.data;
                self.data = nullptr;
                self.dataTypeOffsets.clear();
            };
        }
        else if (findIn(FP_EXT.string(), Asset::EXTS_MODEL.begin(), Asset::EXTS_MODEL.end()))
        {
            // Load model
            asset.info->type = AssetInfo::Type::Model;
            asset.info->onAssetCreate = [fp](AssetInfo& self)
            {
                auto vc = Application::Get().GetWindow().GetVulkanContext();
                auto vr = vc->getRenderer();
                auto data1 = vr->LoadModelFromFile(fp.string());

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
            asset.info->onAssetDestroy = [fp](AssetInfo& self)
            {
                if (self.data)
                {
                    delete self.GetData<ModelData*>();
                    delete self.data;
                }
                self.data = nullptr;
                self.dataTypeOffsets.clear();
            };
        }

        // Call asset creation callback
        asset.createData();

        return asset;
    }
}
