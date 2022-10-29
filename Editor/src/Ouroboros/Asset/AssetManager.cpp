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

#include "Ouroboros/Asset/BinaryIO.h"
#include "Ouroboros/Core/Application.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "Utility/IEqual.h"

namespace oo
{
    bool AssetManager::AssetStore::empty() const
    {
        return all.empty();
    }

    void AssetManager::AssetStore::clear()
    {
        all.clear();
        for (auto& map : byType)
        {
            map.second.clear();
        }
        byType.clear();
    }

    AssetManager::AssetInfoPtr AssetManager::AssetStore::emplace(const AssetID& id, const AssetInfo& info)
    {
        return emplace(id, std::make_shared<AssetInfo>(info));
    }

    AssetManager::AssetInfoPtr AssetManager::AssetStore::emplace(const AssetID& id, AssetInfoPtr ptr)
    {
        all.emplace(id, ptr);
        if (!ptr)
            return ptr;
        if (!byType.contains(ptr->type))
            byType.emplace(ptr->type, AssetInfoMap());
        byType.at(ptr->type).emplace(id, ptr);
        return ptr;
    }

    void AssetManager::AssetStore::erase(const AssetID& id)
    {
        if (!contains(id))
            return;
        auto sp = all.at(id);
        if (sp && byType.contains(sp->type))
            byType.at(sp->type).erase(id);
        all.erase(id);
    }

    std::shared_ptr<AssetInfo>& AssetManager::AssetStore::at(const AssetID& id)
    {
        return all.at(id);
    }

    const std::shared_ptr<AssetInfo>& AssetManager::AssetStore::at(const AssetID& id) const
    {
        return all.at(id);
    }

    bool AssetManager::AssetStore::contains(const AssetID& id) const
    {
        return all.contains(id);
    }

    AssetManager::AssetInfoMap& AssetManager::AssetStore::filter(const AssetInfo::Type& type)
    {
        if (!byType.contains(type))
            byType.at(type) = {};
        return byType.at(type);
    }

    const AssetManager::AssetInfoMap& AssetManager::AssetStore::filter(const AssetInfo::Type& type) const
    {
        return byType.at(type);
    }


    AssetManager::AssetManager(std::filesystem::path root)
        : root{ root }
    {
        EventManager::Subscribe<AssetManager, FileWatchEvent>(this, &AssetManager::watchFiles);
        EventManager::Subscribe<AssetManager, WindowFocusEvent>(this, &AssetManager::windowFocusHandler);
    }

    AssetManager::~AssetManager()
    {
        store.clear();
    }

    Asset AssetManager::Get(const AssetID& id)
    {
        Asset asset;
        if (store.contains(id))
            asset.info = store.at(id);
        return asset;
    }

    std::future<Asset> AssetManager::GetAsync(const AssetID& id)
    {
        return std::async(std::launch::async, &AssetManager::Get, this, id);
    }

    std::vector<Asset> AssetManager::GetLoadedAssetsByType(AssetInfo::Type type) const
    {
        std::vector<Asset> v;
        auto filtered = store.filter(type);
        std::transform(filtered.begin(), filtered.end(), std::back_inserter(v), [this](const decltype(*filtered.begin())& e)
        {
            return Asset(e.second);
        });
        return v;
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
        return std::async(std::launch::async, &AssetManager::LoadDirectory, this, fn, caseSensitive);
    }

    void AssetManager::ReloadAssets()
    {
        FileWatchEvent fwe{ lastReloadTime };
        EventManager::Broadcast<FileWatchEvent>(&fwe);
    }

    void AssetManager::ForceReloadAssets()
    {
        FileWatchEvent fwe{ std::chrono::file_clock::time_point() };
        EventManager::Broadcast<FileWatchEvent>(&fwe);
    }

    void AssetManager::windowFocusHandler(WindowFocusEvent*)
    {
        ReloadAssets();
    }

    void AssetManager::watchFiles(FileWatchEvent* ev)
    {
        TRACY_PROFILE_SCOPE_NC(ASSET_MANAGER_WATCH_FILES, tracy::Color::Aquamarine1);

        try
        {
            std::chrono::file_clock::time_point tLast = ev->time;
            if (std::filesystem::exists(root))
            {
                iterateDirectory(std::filesystem::canonical(root), tLast);
            }
            lastReloadTime = std::chrono::file_clock::now();
        }
        catch (...)
        {
            // do nothing
            // sometimes the path becomes invalidated for unknown, inconsistent reasons
            // so just doing this to abort the file hierarchy update if so
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void AssetManager::iterateDirectory(const std::filesystem::path& dir,
                                        const std::chrono::file_clock::time_point& tLast,
                                        const std::chrono::file_clock::time_point& t)
    {
        // Check if directory was updated recently
        const auto DIR_WRITE_TIME = std::filesystem::last_write_time(dir);
        if (tLast < DIR_WRITE_TIME && DIR_WRITE_TIME <= t)
        {
            LOG_INFO("Iterating {0}", dir);

            for (auto& fp : std::filesystem::directory_iterator(dir))
            {
                const std::filesystem::path FP = std::filesystem::canonical(fp.path());

                // Recurse
                if (std::filesystem::is_directory(FP))
                    iterateDirectory(FP, tLast, t);

                // Check if file
                if (!std::filesystem::is_regular_file(FP))
                    continue;

                // Check if not meta file
                const std::filesystem::path FP_EXT = FP.extension();
                if (FP_EXT == Asset::EXT_META)
                    continue;

                // Ensure meta file exists
                auto fpMeta = FP;
                fpMeta += Asset::EXT_META;
                if (!std::filesystem::exists(fpMeta))
                {
                    ensureMeta(FP);
                }

                // Read meta contents
                AssetMetaContent meta;
                std::ifstream ifs = std::ifstream(fpMeta);
                BinaryIO::Read(ifs, meta);
                const auto WRITE_TIME = std::filesystem::last_write_time(fp.path());
                if (!store.contains(meta.id))
                {
                    // Created
                    LoadPath(FP);
                    LOG_INFO("Load {0}", FP);
                }
                else if (store.at(meta.id)->contentPath != FP)
                {
                    // Moved
                    store.at(meta.id)->contentPath = FP;
                    store.at(meta.id)->metaPath = fpMeta;
                    LOG_INFO("Move {0}", FP);
                }
                else if (tLast < WRITE_TIME && WRITE_TIME <= t)
                {
                    // Modified
                    store.at(meta.id)->contentPath = FP;
                    store.at(meta.id)->metaPath = fpMeta;
                    store.at(meta.id)->timeLoaded = t;
                    store.at(meta.id)->Reload();
                    LOG_INFO("Modify {0}", FP);
                }
            }
        }
    }

    AssetMetaContent AssetManager::ensureMeta(const std::filesystem::path& fp)
    {
        // Get meta path
        auto fpMeta = fp;
        if (fp.extension() != Asset::EXT_META)
        {
            fpMeta += Asset::EXT_META;
        }

        AssetMetaContent meta;
        if (!std::filesystem::exists(fpMeta))
        {
            // Create meta file
            meta.id = Asset::GenerateSnowflake();
            std::ofstream ofs = std::ofstream(fpMeta);
            BinaryIO::Write(ofs, meta);
        }
        else
        {
            // Read meta file
            std::ifstream ifs = std::ifstream(fpMeta);
            BinaryIO::Read(ifs, meta);
        }
        return meta;
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

        // Get or load asset
        AssetMetaContent meta = ensureMeta(fpContent);
        if (store.contains(meta.id))
        {
            // Get asset
            return Asset(store.at(meta.id));
        }
        else
        {
            // Load asset
            return loadAssetIntoStore(fpContent, meta.id);
        }
    }

    Asset AssetManager::loadAssetIntoStore(std::filesystem::path fp, AssetID id)
    {
        AssetInfoPtr info = std::make_shared<AssetInfo>();
        info->id = id;
        info->contentPath = fp;
        info->metaPath = fp; info->metaPath += Asset::EXT_META;
        info->timeLoaded = std::chrono::file_clock::now();
        info->Reload();
        return Asset(store.emplace(info->id, info));
    }

}
