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

namespace
{
    bool isMetaPath(const std::filesystem::path& path)
    {
        return path.extension().string() == oo::Asset::EXT_META;
    }
}
namespace oo
{
    AssetManager::AssetInfoFileTree::Node* AssetManager::AssetInfoFileTree::insert(const std::filesystem::path& path)
    {
        Node* curr = root.get();
        for (const std::filesystem::path& part : path)
        {
            if (part.empty())
                break;
            std::string partStr = part.string();
            if (!curr->children.contains(partStr))
                curr->children.emplace(partStr, std::make_unique<AssetManager::AssetInfoFileTree::Node>(partStr));
            curr = curr->children.at(partStr).get();
        }
        return curr;
    }

    void AssetManager::AssetInfoFileTree::erase(const std::filesystem::path& path)
    {
        Node* prev = nullptr;
        Node* curr = root.get();
        for (const std::filesystem::path& part : path)
        {
            if (part.empty())
                break;
            std::string partStr = part.string();
            if (!curr->children.contains(partStr))
                return;
            prev = curr;
            curr = curr->children.at(partStr).get();
        }
        if (prev)
            prev->children.erase(curr->name);
    }

    AssetManager::AssetInfoFileTree::Node* AssetManager::AssetInfoFileTree::at(const std::filesystem::path& path)
    {
        Node* curr = root.get();
        for (const std::filesystem::path& part : path)
        {
            if (part.empty())
                break;
            std::string partStr = part.string();
            if (!curr->children.contains(partStr))
                return nullptr;
            curr = curr->children.at(partStr).get();
        }
        return curr;
    }

    const AssetManager::AssetInfoFileTree::Node* AssetManager::AssetInfoFileTree::at(const std::filesystem::path& path) const
    {
        Node* curr = root.get();
        for (const std::filesystem::path& part : path)
        {
            if (part.empty())
                break;
            std::string partStr = part.string();
            if (!curr->children.contains(partStr))
                return nullptr;
            curr = curr->children.at(partStr).get();
        }
        return curr;
    }

    bool AssetManager::AssetInfoFileTree::contains(const std::filesystem::path& path) const
    {
        return at(path);
    }



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
        // Insert into maps
        all.emplace(id, ptr);
        if (!ptr)
            return ptr;
        byType.at(ptr->type).emplace(id, ptr);

        // Insert into fs tree
        tree.insert(ptr->contentPath)->id = id;

        return ptr;
    }

    void AssetManager::AssetStore::erase(const AssetID& id)
    {
        // Remove from maps
        if (!contains(id))
            return;
        auto sp = all.at(id);
        if (sp)
            byType.at(sp->type).erase(id);
        all.erase(id);

        // Remove from fs tree
        tree.erase(sp->contentPath);
    }

    std::shared_ptr<AssetInfo>& AssetManager::AssetStore::at(const AssetID& id)
    {
        return all.at(id);
    }

    const std::shared_ptr<AssetInfo>& AssetManager::AssetStore::at(const AssetID& id) const
    {
        return all.at(id);
    }

    AssetManager::AssetInfoFileTree::Node* AssetManager::AssetStore::at(const std::filesystem::path& path)
    {
        return tree.at(path);
    }

    const AssetManager::AssetInfoFileTree::Node* AssetManager::AssetStore::at(const std::filesystem::path& path) const
    {
        return tree.at(path);
    }

    bool AssetManager::AssetStore::contains(const AssetID& id) const
    {
        return all.contains(id);
    }

    bool AssetManager::AssetStore::contains(const std::filesystem::path& path) const
    {
        return tree.contains(path);
    }

    AssetManager::AssetInfoMap& AssetManager::AssetStore::filter(const AssetInfo::Type& type)
    {
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
        for (int i = 0; i < static_cast<int>(AssetInfo::Type::_COUNT); ++i)
            store.byType.emplace(static_cast<AssetInfo::Type>(i), AssetInfoMap());
    }

    AssetManager::~AssetManager()
    {
        EventManager::Unsubscribe<AssetManager, WindowFocusEvent>(this, &AssetManager::windowFocusHandler);
        EventManager::Unsubscribe<AssetManager, FileWatchEvent>(this, &AssetManager::watchFiles);
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

    std::vector<Asset> AssetManager::GetAssetsByType(AssetInfo::Type type) const
    {
        std::vector<Asset> v;
        auto filtered = store.filter(type);
        if (filtered.empty())
            return v;
        std::transform(filtered.begin(), filtered.end(), std::back_inserter(v), [this](const decltype(filtered)::value_type& e)
        {
            return Asset(e.second);
        });
        return v;
    }

    Asset AssetManager::GetOrLoadPath(const std::filesystem::path& fp)
    {
        const auto FP = root / fp;

        if (!std::filesystem::exists(FP))
            return Asset();

        return getLoadedAsset(FP);
    }

    std::future<Asset> AssetManager::GetOrLoadPathAsync(const std::filesystem::path& fp)
    {
        return std::async(std::launch::async, &AssetManager::GetOrLoadPath, this, fp);
    }

    std::vector<Asset> AssetManager::GetOrLoadDirectory(const std::filesystem::path& path, bool recursive)
    {
        const auto PATH = root / path;

        if (!std::filesystem::exists(PATH) || !std::filesystem::is_directory(PATH))
            return {};

        std::vector<std::filesystem::path> files;
        if (recursive)
        {
            for (auto& file : std::filesystem::recursive_directory_iterator(PATH))
            {
                if (isMetaPath(file.path()))
                    continue;
                files.emplace_back(file);
            }
        }
        else
        {
            for (auto& file : std::filesystem::directory_iterator(PATH))
            {
                if (isMetaPath(file.path()))
                    continue;
                files.emplace_back(file);
            }
        }
        std::vector<Asset> v;
        for (auto& file : files)
        {
            v.emplace_back(getLoadedAsset(file));
        }
        return v;
    }

    std::future<std::vector<Asset>> AssetManager::GetOrLoadDirectoryAsync(const std::filesystem::path& path, bool recursive)
    {
        return std::async(std::launch::async, &AssetManager::GetOrLoadDirectory, this, path, recursive);
    }

    std::vector<Asset> AssetManager::GetOrLoadName(const std::filesystem::path& fn, bool caseSensitive)
    {
        const std::filesystem::path DIR = std::filesystem::canonical(root);

        std::vector<Asset> v;
        for (auto& file : std::filesystem::recursive_directory_iterator(DIR))
        {
            if ((caseSensitive && file.path().filename() == fn) ||
                (!caseSensitive && iequal(file.path().filename().string(), fn.string())))
            {
                v.emplace_back(getLoadedAsset(file.path()));
            }
        }
        return v;
    }

    std::future<std::vector<Asset>> AssetManager::GetOrLoadNameAsync(const std::filesystem::path& fn, bool caseSensitive)
    {
        return std::async(std::launch::async, &AssetManager::GetOrLoadDirectory, this, fn, caseSensitive);
    }

    void AssetManager::UnloadAll()
    {
        for (auto& i : store.all)
            i.second->Unload();
    }

    void AssetManager::Scan()
    {
        FileWatchEvent fwe{ lastReloadTime };
        EventManager::Broadcast<FileWatchEvent>(&fwe);
    }

    void AssetManager::ForceScan()
    {
        FileWatchEvent fwe{ std::chrono::file_clock::time_point() };
        EventManager::Broadcast<FileWatchEvent>(&fwe);
    }

    void AssetManager::windowFocusHandler(WindowFocusEvent*)
    {
        Scan();
    }

    void AssetManager::watchFiles(FileWatchEvent* ev)
    {
        TRACY_PROFILE_SCOPE_NC(ASSET_MANAGER_WATCH_FILES, tracy::Color::Aquamarine1);

        std::chrono::file_clock::time_point tLast = ev->time;
        if (std::filesystem::exists(root))
        {
            iterateDirectory(root, tLast);
        }
        lastReloadTime = std::chrono::file_clock::now();

        TRACY_PROFILE_SCOPE_END();
    }

    void AssetManager::iterateDirectory(const std::filesystem::path& dir,
                                        const std::chrono::file_clock::time_point& tLast,
                                        const std::chrono::file_clock::time_point& t)
    {
        // Check if directory was updated recently
        const std::filesystem::path DIR = std::filesystem::canonical(dir);
        const auto DIR_WRITE_TIME = std::filesystem::last_write_time(dir);
        if (tLast < DIR_WRITE_TIME && DIR_WRITE_TIME <= t)
        {
            LOG_INFO("Iterating {0}", DIR);

            auto node = store.at(DIR);
            if (node)
            {
                for (const auto& asset : node->children)
                {
                    const std::filesystem::path FP = DIR / asset.second->name;

                    // Check if file in tree no longer exists
                    if (std::filesystem::exists(FP))
                        continue;

                    // Removed
                    store.erase(asset.second->id);
                    LOG_INFO("De-indexed asset {0}", FP.filename());
                }
            }

            for (auto& fp : std::filesystem::directory_iterator(DIR))
            {
                const std::filesystem::path FP = std::filesystem::canonical(fp.path());

                // Recurse
                if (std::filesystem::is_directory(FP))
                    iterateDirectory(FP, tLast, t);

                // Check if not meta file
                if (isMetaPath(FP))
                    continue;

                // Ensure meta file exists
                auto fpMeta = FP;
                fpMeta += Asset::EXT_META;
                AssetMetaContent meta = ensureMeta(fpMeta);

                // Check if file
                if (!std::filesystem::is_regular_file(FP))
                    continue;

                // Read meta contents
                const auto WRITE_TIME = std::filesystem::last_write_time(fp.path());
                if (!store.contains(meta.id))
                {
                    // Created
                    GetOrLoadPath(FP);
                }
                //else if (store.at(meta.id)->contentPath != FP)
                //{
                //    // Moved
                //    store.at(meta.id)->contentPath = FP;
                //    store.at(meta.id)->metaPath = fpMeta;
                //    LOG_INFO("Move {0}", FP);
                //}
                else if (tLast < WRITE_TIME && WRITE_TIME <= t)
                {
                    // Modified
                    store.at(meta.id)->contentPath = FP;
                    store.at(meta.id)->metaPath = fpMeta;
                    store.at(meta.id)->timeLoaded = t;
                    store.at(meta.id)->Reload();
                    LOG_INFO("Modified asset {0}", FP.filename());
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
            LOG_INFO("Created meta {0}", fpMeta.filename());
        }
        else
        {
            // Read meta file
            std::ifstream ifs = std::ifstream(fpMeta);
            BinaryIO::Read(ifs, meta);
        }
        return meta;
    }

    Asset AssetManager::getAsset(const std::filesystem::path& fp)
    {
        // Get file paths
        auto fpContent = fp;
        auto fpMeta = fpContent;
        if (isMetaPath(fpContent))
        {
            fpContent.replace_extension();
        }
        else
        {
            fpMeta += Asset::EXT_META;
        }
        const auto FP_EXT = fpContent.extension();

        // Get or index asset
        AssetMetaContent meta = ensureMeta(fpContent);
        if (store.contains(meta.id))
        {
            // Get asset
            return Asset(store.at(meta.id));
        }
        else
        {
            // Index asset
            return indexAsset(fpContent, meta.id);
        }
    }

    Asset AssetManager::getLoadedAsset(const std::filesystem::path& fp)
    {
        Asset asset = getAsset(fp);
        if (!asset.IsDataLoaded())
            asset.Reload();
        return asset;
    }

    Asset AssetManager::indexAsset(const std::filesystem::path& fp, AssetID id)
    {
        AssetInfoPtr info = std::make_shared<AssetInfo>();
        info->id = id;
        info->contentPath = std::filesystem::canonical(fp);
        info->metaPath = std::filesystem::canonical(fp); info->metaPath += Asset::EXT_META;
        info->timeLoaded = std::chrono::file_clock::now();
        info->type = info->GetType();
        LOG_INFO("Indexed asset {0}", fp.filename());
        return Asset(store.emplace(info->id, info));
    }
}
