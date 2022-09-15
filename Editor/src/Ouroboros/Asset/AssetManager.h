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
#include <optional>
#include <thread>
#include <unordered_map>
#include <set>
#include <vector>

#include "Asset.h"

namespace oo
{
    class AssetManager
    {
    public:
        /* --------------------------------------------------------------------------- */
        /* Type Definitions                                                            */
        /* --------------------------------------------------------------------------- */

        using AssetMap = std::unordered_map<AssetID, Asset>;
        using TypeAssetIDsMap = std::unordered_map<AssetInfo::Type, std::set<AssetID>>;
        class AssetStore
        {
        public:
            /* ----------------------------------------------------------------------- */
            /* Getters                                                                 */
            /* ----------------------------------------------------------------------- */

            [[nodiscard]] inline const AssetMap& GetAssets() const { return assets; };

            /* ----------------------------------------------------------------------- */
            /* Functions                                                               */
            /* ----------------------------------------------------------------------- */

            Asset& At(AssetID id);
            Asset At(AssetID id) const;
            std::vector<std::reference_wrapper<Asset>> At(AssetInfo::Type type);
            std::vector<Asset> At(AssetInfo::Type type) const;
            Asset Insert(AssetID id, const Asset& asset);
            void Erase(AssetID id);
            bool Contains(AssetID id) const;

        private:
            /* ----------------------------------------------------------------------- */
            /* Members                                                                 */
            /* ----------------------------------------------------------------------- */

            AssetMap assets;
            TypeAssetIDsMap assetsByType;
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
        [[nodiscard]] inline const AssetMap& GetAssets() const { return assets.GetAssets(); };

        /* --------------------------------------------------------------------------- */
        /* Setters                                                                     */
        /* --------------------------------------------------------------------------- */

        [[nodiscard]] inline std::filesystem::path& RootDirectory() { return root; };

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Retrieves an asset using its ID.
        /// </summary>
        /// <param name="snowflake">The ID of the asset.</param>
        /// <returns>The asset.</returns>
        Asset Get(const AssetID& snowflake);

        /// <summary>
        /// Asynchronously retrieves an asset using its ID.
        /// </summary>
        /// <param name="snowflake">The ID of the asset.</param>
        /// <returns>The future asset.</returns>
        std::future<Asset> GetAsync(const AssetID& snowflake);

        /// <summary>
        /// Retrieves all loaded assets of a given type.
        /// </summary>
        /// <param name="type">The type of asset.</param>
        /// <returns>The loaded assets.</returns>
        std::vector<Asset> GetLoadedAssetsByType(AssetInfo::Type type) const;

        /// <summary>
        /// Loads or retrieves an asset at a given file path.
        /// </summary>
        /// <param name="fp">The file path relative to the AssetManager's root path.</param>
        /// <returns>The asset.</returns>
        Asset LoadPath(const std::filesystem::path& fp);

        /// <summary>
        /// Asynchronously loads or retrieves an asset at a given file path.
        /// </summary>
        /// <param name="fp">The file path relative to the AssetManager's root path.</param>
        /// <returns>The future asset.</returns>
        std::future<Asset> LoadPathAsync(const std::filesystem::path& fp);

        /// <summary>
        /// Loads or retrieves all assets in a given directory path.
        /// </summary>
        /// <param name="path">The directory path relative to the AssetManager's root path.</param>
        /// <param name="recursive">Whether to load directories recursively.</param>
        /// <returns>The assets.</returns>
        std::vector<Asset> LoadDirectory(const std::filesystem::path& path, bool recursive = false);

        /// <summary>
        /// Loads or retrieves all assets in a given directory path.
        /// </summary>
        /// <param name="path">The directory path relative to the AssetManager's root path.</param>
        /// <param name="recursive">Whether to load directories recursively.</param>
        /// <returns>The assets.</returns>
        std::future<std::vector<Asset>> LoadDirectoryAsync(const std::filesystem::path& path, bool recursive = false);

        /// <summary>
        /// Loads or retrieves an asset by a given file name.
        /// </summary>
        /// <param name="fn">The file name.</param>
        /// <param name="caseSensitive">Whether the file name is case sensitive.</param>
        /// <returns>The assets matching the criteria.</returns>
        std::vector<Asset> LoadName(const std::filesystem::path& fn, bool caseSensitive = true);

        /// <summary>
        /// Asynchronously loads or retrieves an asset by a given file name.
        /// </summary>
        /// <param name="fn">The file name.</param>
        /// <param name="caseSensitive">Whether the file name is case sensitive.</param>
        /// <returns>The assets matching the criteria.</returns>
        std::future<std::vector<Asset>> LoadNameAsync(const std::filesystem::path& fn, bool caseSensitive = true);

    private:
        /* --------------------------------------------------------------------------- */
        /* Members                                                                     */
        /* --------------------------------------------------------------------------- */

        bool isRunning = true;
        std::filesystem::path root;
        AssetStore assets;
        std::thread fileWatchThread;

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Scans the filesystem for changes in files.
        /// </summary>
        void fileWatch();

        /// <summary>
        /// Recursively update asset paths inside a directory.
        /// </summary>
        /// <param name="dir">The directory.</param>
        void updateAssetPaths(const std::filesystem::path& dir);

        /// <summary>
        /// Loads or retrieves an asset at a given absolute file path.
        /// </summary>
        /// <param name="fp">The file path.</param>
        /// <returns>The asset.</returns>
        Asset getOrLoadAbsolute(const std::filesystem::path& fp);

        /// <summary>
        /// Creates an asset object from a given file.
        /// </summary>
        /// <param name="fp">The file path.</param>
        /// <returns>The asset.</returns>
        Asset createAsset(std::filesystem::path fp);
    };

    class AssetNotFoundException : public std::exception
    {
    public:
        AssetNotFoundException(const std::string& what = "") : std::exception(what.c_str()) {}
    };
}
