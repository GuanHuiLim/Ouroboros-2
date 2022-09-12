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
        [[nodiscard]] inline const AssetMap& GetAssets() const { return assets; };

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
        /// <returns>The assets.</returns>
        std::vector<Asset> LoadDirectory(const std::filesystem::path& path);

        /// <summary>
        /// Loads or retrieves all assets in a given directory path.
        /// </summary>
        /// <param name="path">The directory path relative to the AssetManager's root path.</param>
        /// <returns>The assets.</returns>
        std::future<std::vector<Asset>> LoadDirectoryAsync(const std::filesystem::path& path);

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
        AssetMap assets;
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
