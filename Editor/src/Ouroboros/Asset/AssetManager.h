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
        /// Loads or retrieves an asset by a given file name.
        /// </summary>
        /// <param name="fn">The file name.</param>
        /// <returns>The assets matching the criteria.</returns>
        std::vector<Asset> LoadName(const std::filesystem::path& fn);
        
        /// <summary>
        /// Asynchronously loads or retrieves an asset by a given file name.
        /// </summary>
        /// <param name="fn">The file name.</param>
        /// <returns>The assets matching the criteria.</returns>
        std::future<std::vector<Asset>> LoadNameAsync(const std::filesystem::path& fn);

    private:
        /* --------------------------------------------------------------------------- */
        /* Members                                                                     */
        /* --------------------------------------------------------------------------- */

        bool isRunning = true;
        std::filesystem::path root;
        std::unordered_map<AssetID, Asset> assets;
        std::thread fileWatchThread;

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Scans the filesystem for changes in files.
        /// </summary>
        void fileWatch();

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
