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
        GetDirectory(root, true);
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
        std::for_each(filtered.begin(), filtered.end(), [&v](const decltype(filtered)::value_type& e)
        {
            auto asset = Asset(e.second);
            v.insert(std::upper_bound(v.begin(), v.end(), asset, [](const Asset& a, const Asset& b)
            {
                return a.GetFilePath().filename().string() < b.GetFilePath().filename().string();
            }), asset);
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

#define DIR_ITER(_CALLBACK)                                                                         \
if (recursive) for (auto& file : std::filesystem::recursive_directory_iterator(PATH)) { _CALLBACK; }\
else for (auto& file : std::filesystem::directory_iterator(PATH)) { _CALLBACK; }

    std::vector<Asset> AssetManager::GetDirectory(const std::filesystem::path& path, bool recursive)
    {
        const auto PATH = root / path;

        if (!std::filesystem::exists(PATH) || !std::filesystem::is_directory(PATH))
            return {};

        std::vector<std::filesystem::path> files;
        DIR_ITER({ if (isMetaPath(file.path())) continue; files.emplace_back(file); });
        std::vector<Asset> v;
        std::transform(files.begin(), files.end(), std::back_inserter(v), [this](const auto& file) { return getAsset(file); });
        return v;
    }

    std::future<std::vector<Asset>> AssetManager::GetDirectoryAsync(const std::filesystem::path& path, bool recursive)
    {
        return std::async(std::launch::async, &AssetManager::GetDirectory, this, path, recursive);
    }

    std::vector<Asset> AssetManager::GetOrLoadDirectory(const std::filesystem::path& path, bool recursive)
    {
        const auto PATH = root / path;

        if (!std::filesystem::exists(PATH) || !std::filesystem::is_directory(PATH))
            return {};

        std::vector<std::filesystem::path> files;
        DIR_ITER({ if (isMetaPath(file.path())) continue; files.emplace_back(file); });
        std::vector<Asset> v;
        std::transform(files.begin(), files.end(), std::back_inserter(v), [this](const auto& file) { return getLoadedAsset(file); });
        return v;
    }

    std::future<std::vector<Asset>> AssetManager::GetOrLoadDirectoryAsync(const std::filesystem::path& path, bool recursive)
    {
        return std::async(std::launch::async, &AssetManager::GetOrLoadDirectory, this, path, recursive);
    }

#undef DIR_ITER

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
            iterateDirectoryOrphans(root, tLast);
            iterateDirectoryAdditions(root, tLast);
            iterateDirectoryOmissions(root, tLast);
        }
        lastReloadTime = std::chrono::file_clock::now();

        TRACY_PROFILE_SCOPE_END();
    }

    void AssetManager::iterateDirectoryOrphans(const std::filesystem::path& dir,
                                               const std::chrono::file_clock::time_point& tLast,
                                               const std::chrono::file_clock::time_point& t)
    {
        const std::filesystem::path DIR = std::filesystem::canonical(dir);
        std::list<std::filesystem::path> toRemove;

        // Iterate directory contents
        for (auto& fp : std::filesystem::directory_iterator(DIR))
        {
            const std::filesystem::path FP = std::filesystem::canonical(fp.path());

            // Recurse
            if (std::filesystem::is_directory(FP))
                iterateDirectoryOrphans(FP, tLast, t);

            // Check if meta file
            if (!isMetaPath(FP))
                continue;

            // Convert to asset path
            std::filesystem::path fpContent = FP;
            fpContent.replace_extension();

            // Check if item exists
            if (std::filesystem::exists(fpContent))
                continue;

            toRemove.emplace_back(FP);
        }

        // Remove orphans
        for (const auto& fp : toRemove)
        {
            std::filesystem::remove(fp);
            LOG_WARN("Removed orphaned meta file {0}", fp.filename());
        }
    }

    void AssetManager::iterateDirectoryAdditions(const std::filesystem::path& dir,
                                                 const std::chrono::file_clock::time_point& tLast,
                                                 const std::chrono::file_clock::time_point& t)
    {
        const std::filesystem::path DIR = std::filesystem::canonical(dir);
        const auto DIR_WRITE_TIME = std::filesystem::last_write_time(DIR);

        // Iterate directory contents
        for (auto& fp : std::filesystem::directory_iterator(DIR))
        {
            const std::filesystem::path FP = std::filesystem::canonical(fp.path());

            // Recurse
            if (std::filesystem::is_directory(FP))
                iterateDirectoryAdditions(FP, tLast, t);

            // Check if directory was updated recently
            if (tLast < DIR_WRITE_TIME && DIR_WRITE_TIME <= t)
            {
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
                    getLoadedAsset(FP);
                }
                else if (store.at(meta.id)->contentPath != FP)
                {
                    // Moved
                    const auto FP_OLD = store.at(meta.id)->contentPath;
                    store.tree.erase(FP_OLD);
                    store.at(meta.id)->contentPath = FP;
                    store.at(meta.id)->metaPath = fpMeta;
                    store.at(meta.id)->Reload();
                    store.tree.insert(FP);
                    //LOG_INFO("Move {0}", FP.filename());
                }
                else if (tLast < WRITE_TIME && WRITE_TIME <= t)
                {
                    // Modified
                    store.at(meta.id)->contentPath = FP;
                    store.at(meta.id)->metaPath = fpMeta;
                    store.at(meta.id)->timeLoaded = t;
                    store.at(meta.id)->Reload();
                    //LOG_INFO("Modified asset {0}", FP.filename());
                }
            }
        }
    }

    void AssetManager::iterateDirectoryOmissions(const std::filesystem::path& dir,
                                                 const std::chrono::file_clock::time_point& tLast,
                                                 const std::chrono::file_clock::time_point& t)
    {
        const std::filesystem::path DIR = std::filesystem::canonical(dir);

        // Iterate directory contents
        for (auto& fp : std::filesystem::directory_iterator(DIR))
        {
            const std::filesystem::path FP = std::filesystem::canonical(fp.path());

            // Recurse
            if (std::filesystem::is_directory(FP))
                iterateDirectoryOmissions(FP, tLast, t);

            // Check if directory was updated recently
            const auto DIR_WRITE_TIME = std::filesystem::last_write_time(DIR);
            if (tLast < DIR_WRITE_TIME && DIR_WRITE_TIME <= t)
            {
                // Get tree node
                std::vector<AssetID> toRemove;
                auto node = store.at(DIR);
                if (node)
                {
                    // Iterate tree node children
                    for (const auto& asset : node->children)
                    {
                        const std::filesystem::path ASS_FP = DIR / asset.second->name; // haha ass

                        // Check if file in tree no longer exists
                        if (std::filesystem::exists(ASS_FP))
                            continue;

                        // Remove
                        toRemove.emplace_back(asset.second->id);
                        //LOG_INFO("Un-indexed asset {0}", ASS_FP.filename());
                    }
                }
                for (const auto& i : toRemove)
                {
                    store.erase(i);
                }
            }
        }
    }

    AssetMetaContent AssetManager::ensureMeta(const std::filesystem::path& fp)
    {
        auto [fpContent, fpMeta, fpExt] = getAssetPathParts(fp);
        AssetMetaContent meta;
        if (!std::filesystem::exists(fpMeta))
        {
            // Create meta file
            meta.id = Asset::GenerateSnowflake();
            std::ofstream ofs = std::ofstream(fpMeta, std::ios::binary);
            BinaryIO::Write(ofs, meta);
            //LOG_INFO("Created meta {0}", fpMeta.filename());
        }
        else
        {
            // Read meta file
            std::ifstream ifs = std::ifstream(fpMeta, std::ios::binary);
            BinaryIO::Read(ifs, meta);
        }
        return meta;
    }

    std::mutex storeMutex;

    Asset AssetManager::indexAsset(const std::filesystem::path& fp, AssetID id)
    {
        //LOG_INFO("Indexed asset {0}", fp.filename());
        std::lock_guard<std::mutex> guard(storeMutex);

        AssetInfoPtr info = std::make_shared<AssetInfo>();
        info->id = id;
        info->contentPath = std::filesystem::canonical(fp);
        info->metaPath = std::filesystem::canonical(fp); info->metaPath += Asset::EXT_META;
        info->timeLoaded = std::chrono::file_clock::now();
        info->type = info->GetType();
        //LOG_INFO("Indexed asset {0}", fp.filename());
        return Asset(store.emplace(info->id, info));
    }

    Asset AssetManager::getAsset(const std::filesystem::path& fp)
    {
        auto [fpContent, fpMeta, fpExt] = getAssetPathParts(fp);

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

    std::tuple<std::filesystem::path, std::filesystem::path, std::filesystem::path> AssetManager::getAssetPathParts(const std::filesystem::path& fp)
    {
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
        return std::tuple(fpContent, fpMeta, FP_EXT);
    }
}
