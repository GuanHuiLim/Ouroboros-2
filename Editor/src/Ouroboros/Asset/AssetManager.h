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
        static constexpr size_t WATCH_INTERVAL = 1000;

        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* -------------------------------------------------------------------------- -*/

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

        /****************************************************************************//*!
        @brief  Retrieves an asset using its ID.
        @param  snowflake - The ID of the asset.
        @return The asset.
        *//*****************************************************************************/
        Asset Get(const AssetID& snowflake);

        /****************************************************************************//*!
        @brief  Asynchronously retrieves an asset using its ID.
        @param  snowflake - The ID of the asset.
        @return The future asset.
        *//*****************************************************************************/
        std::future<Asset> GetAsync(const AssetID& snowflake);

        /****************************************************************************//*!
        @brief  Loads or retrieves an asset at a given file path.
        @param  fp - The file path relative to the AssetManager's root path.
        @return The future asset.
        *//*****************************************************************************/
        Asset LoadPath(const std::filesystem::path& fp);

        /****************************************************************************//*!
        @brief  Asynchronously loads or retrieves an asset at a given file path.
        @param  fp - The file path relative to the AssetManager's root path.
        @return The future asset.
        *//*****************************************************************************/
        std::future<Asset> LoadPathAsync(const std::filesystem::path& fp);

    private:
        bool isRunning = true;
        std::filesystem::path root;
        std::unordered_map<AssetID, Asset> assets;
        std::thread fileWatchThread;

        /****************************************************************************//*!
        @brief  Scans the filesystem for changes in files.
        *//*****************************************************************************/
        void fileWatch();

        /****************************************************************************//*!
        @brief  Creates an asset object from a given file.
        @param  fp - The file path.
        @return The asset.
        *//*****************************************************************************/
        Asset createAsset(std::filesystem::path fp);
    };

    class AssetNotFoundException : public std::exception
    {
    public:
        AssetNotFoundException(const std::string& what = "") : std::exception(what.c_str()) {}
    };
}
