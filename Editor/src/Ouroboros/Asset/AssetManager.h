/************************************************************************************//*!
\file           AssetManager.h
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Aug 30, 2022
\brief          Contains the declaration for the AssetManager class.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <optional>
#include <thread>
#include <unordered_map>
#include <set>
#include <vector>

#include "Asset.h"

#include "Ouroboros/Core/Events/ApplicationEvent.h"
#include "Ouroboros/EventSystem/Event.h"

class FileWatchEvent :public oo::Event
{
public:
    FileWatchEvent(const std::chrono::file_clock::time_point& t) : time{ t } {};
    ~FileWatchEvent() {};

    std::chrono::file_clock::time_point time;
};

namespace oo
{
    class AssetManager
    {
    public:
        /* --------------------------------------------------------------------------- */
        /* Type Definitions                                                            */
        /* --------------------------------------------------------------------------- */

        using AssetInfoPtr = std::shared_ptr<AssetInfo>;
        using AssetInfoMap = std::unordered_map<AssetID, AssetInfoPtr>;
        using AssetInfoTypedMap = std::unordered_map<AssetInfo::Type, AssetInfoMap>;

        struct AssetInfoFileTree
        {
            struct Node
            {
                using Container = std::map<std::string, std::unique_ptr<Node>>;
                Node(const std::string& name = "", AssetID id = Asset::ID_NULL) : name{ name }, id{ id } {}
                std::string name;
                AssetID id;
                Container children;
            };

            Node* insert(const std::filesystem::path& path);
            void erase(const std::filesystem::path& path);
            Node* at(const std::filesystem::path& path);
            const Node* at(const std::filesystem::path& path) const;
            bool contains(const std::filesystem::path& path) const;
            void print()
            {
                print(root.get());
            }
            void print(Node* n, int level = 0)
            {
                for (int i = 0; i < level; ++i)
                    std::cout << "  ";
                std::cout << (n->name.empty() ? "<no-name>" : n->name) << std::endl;
                for (auto& i : n->children)
                    print(i.second.get(), level + 1);
            }

            std::unique_ptr<Node> root = std::make_unique<Node>("<ROOT_NODE>");
        };

        struct AssetStore
        {
            bool empty() const;
            void clear();
            AssetInfoPtr emplace(const AssetID& id, const AssetInfo& info);
            AssetInfoPtr emplace(const AssetID& id, AssetInfoPtr ptr);
            void erase(const AssetID& id);
            AssetInfoPtr& at(const AssetID& id);
            const AssetInfoPtr& at(const AssetID& id) const;
            AssetInfoFileTree::Node* at(const std::filesystem::path& path);
            const AssetInfoFileTree::Node* at(const std::filesystem::path& path) const;
            bool contains(const AssetID& id) const;
            bool contains(const std::filesystem::path& path) const;
            AssetInfoMap& filter(const AssetInfo::Type& type);
            const AssetInfoMap& filter(const AssetInfo::Type& type) const;

            AssetInfoMap all;
            AssetInfoTypedMap byType;
            AssetInfoFileTree tree;
        };

        /* --------------------------------------------------------------------------- */
        /* Constants                                                                   */
        /* --------------------------------------------------------------------------- */

        static constexpr size_t WATCH_INTERVAL = 1000;

        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* --------------------------------------------------------------------------- */

        AssetManager(std::filesystem::path root);
        AssetManager(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = default;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager& operator=(AssetManager&&) = default;
        ~AssetManager();

        /* --------------------------------------------------------------------------- */
        /* Getters                                                                     */
        /* --------------------------------------------------------------------------- */

        [[nodiscard]] inline const std::filesystem::path& GetRootDirectory() const { return root; };

        /* --------------------------------------------------------------------------- */
        /* Setters                                                                     */
        /* --------------------------------------------------------------------------- */

        [[nodiscard]] inline std::filesystem::path& RootDirectory() { return root; };

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

#ifdef OO_DEBUG
        void PrintTree()
        {
            store.tree.print();
        }
#endif

        /// <summary>
        /// Retrieves an asset using its ID.
        /// Does not load the asset's associated file data.
        /// </summary>
        /// <param name="snowflake">The ID of the asset.</param>
        /// <returns>The asset.</returns>
        Asset Get(const AssetID& id);

        /// <summary>
        /// Asynchronously retrieves an asset using its ID.
        /// Does not load the asset's associated file data.
        /// </summary>
        /// <param name="snowflake">The ID of the asset.</param>
        /// <returns>The future asset.</returns>
        std::future<Asset> GetAsync(const AssetID& id);

        /// <summary>
        /// Retrieves all loaded assets of a given type.
        /// Does not load the assets' associated file data.
        /// </summary>
        /// <param name="type">The type of asset.</param>
        /// <returns>The loaded assets.</returns>
        std::vector<Asset> GetAssetsByType(AssetInfo::Type type) const;

        /// <summary>
        /// Loads and retrieves an asset at a given file path.
        /// </summary>
        /// <param name="fp">The file path relative to the AssetManager's root path.</param>
        /// <returns>The asset.</returns>
        Asset GetOrLoadPath(const std::filesystem::path& fp);

        /// <summary>
        /// Asynchronously loads and retrieves an asset at a given file path.
        /// </summary>
        /// <param name="fp">The file path relative to the AssetManager's root path.</param>
        /// <returns>The future asset.</returns>
        std::future<Asset> GetOrLoadPathAsync(const std::filesystem::path& fp);

        /// <summary>
        /// Loads and retrieves all assets in a given directory path.
        /// </summary>
        /// <param name="path">The directory path relative to the AssetManager's root path.</param>
        /// <param name="recursive">Whether to load directories recursively.</param>
        /// <returns>The assets.</returns>
        std::vector<Asset> GetOrLoadDirectory(const std::filesystem::path& path, bool recursive = false);

        /// <summary>
        /// Loads or retrieves all assets in a given directory path.
        /// </summary>
        /// <param name="path">The directory path relative to the AssetManager's root path.</param>
        /// <param name="recursive">Whether to load directories recursively.</param>
        /// <returns>The assets.</returns>
        std::future<std::vector<Asset>> GetOrLoadDirectoryAsync(const std::filesystem::path& path, bool recursive = false);

        /// <summary>
        /// Loads or retrieves an asset by a given file name.
        /// </summary>
        /// <param name="fn">The file name.</param>
        /// <param name="caseSensitive">Whether the file name is case sensitive.</param>
        /// <returns>The assets matching the criteria.</returns>
        std::vector<Asset> GetOrLoadName(const std::filesystem::path& fn, bool caseSensitive = true);

        /// <summary>
        /// Asynchronously loads or retrieves an asset by a given file name.
        /// </summary>
        /// <param name="fn">The file name.</param>
        /// <param name="caseSensitive">Whether the file name is case sensitive.</param>
        /// <returns>The assets matching the criteria.</returns>
        std::future<std::vector<Asset>> GetOrLoadNameAsync(const std::filesystem::path& fn, bool caseSensitive = true);

        /// <summary>
        /// Unloads all assets.
        /// </summary>
        void UnloadAll();

        /// <summary>
        /// Scans for updated assets.
        /// </summary>
        void Scan();

        /// <summary>
        /// Scans all assets.
        /// </summary>
        void ForceScan();

    private:
        /* --------------------------------------------------------------------------- */
        /* Members                                                                     */
        /* --------------------------------------------------------------------------- */

        std::filesystem::path root;
        AssetStore store;
        std::chrono::file_clock::time_point lastReloadTime;

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Handles the window focus event.
        /// </summary>
        /// <param name="ev">The window focus event.</param>
        void windowFocusHandler(WindowFocusEvent*);

        /// <summary>
        /// Scans the filesystem for changes in files.
        /// </summary>
        /// <param name="ev">The file watch event.</param>
        void watchFiles(FileWatchEvent* ev);

        /// <summary>
        /// Iterates through a directory for additions and modifications in the filesystem.
        /// </summary>
        /// <param name="dir">The directory to iterate.</param>
        /// <param name="lastTime">The last time at which an iteration was performed.</param>
        /// <param name="iterationTime">The time at which this iteration is performed.</param>
        void iterateDirectoryAdditions(const std::filesystem::path& dir,
                                       const std::chrono::file_clock::time_point& lastTime,
                                       const std::chrono::file_clock::time_point& iterationTime = std::chrono::file_clock::now());

        /// <summary>
        /// Iterates through a directory for omissions in the filesystem.
        /// </summary>
        /// <param name="dir">The directory to iterate.</param>
        /// <param name="lastTime">The last time at which an iteration was performed.</param>
        /// <param name="iterationTime">The time at which this iteration is performed.</param>
        void iterateDirectoryOmissions(const std::filesystem::path& dir,
                                       const std::chrono::file_clock::time_point& lastTime,
                                       const std::chrono::file_clock::time_point& iterationTime = std::chrono::file_clock::now());

        /// <summary>
        /// Ensures that a meta file for an asset exists
        /// </summary>
        /// <param name="fp">The file path of the asset.</param>
        AssetMetaContent ensureMeta(const std::filesystem::path& fp);

        /// <summary>
        /// Retrieves an asset at a given absolute file path.
        /// </summary>
        /// <param name="fp">The file path.</param>
        /// <returns>The asset.</returns>
        Asset getAsset(const std::filesystem::path& fp);

        /// <summary>
        /// Loads and retrieves asset at a given absolute file path.
        /// </summary>
        /// <param name="fp">The file path.</param>
        /// <returns>The asset.</returns>
        Asset getLoadedAsset(const std::filesystem::path& fp);

        /// <summary>
        /// Indexes an asset from a given file into the store.
        /// </summary>
        /// <param name="fp">The file path.</param>
        /// <param name="id">The asset ID.</param>
        /// <returns>The asset.</returns>
        Asset indexAsset(const std::filesystem::path& fp, AssetID id = Asset::GenerateSnowflake());

        friend Asset;
    };

    class AssetNotFoundException : public std::exception
    {
    public:
        AssetNotFoundException(const std::string& what = "Asset Not Found") : std::exception(what.c_str()) {}
    };
}
