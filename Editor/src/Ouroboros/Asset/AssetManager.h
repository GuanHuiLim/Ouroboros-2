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

        AssetManager(std::filesystem::path root);
        AssetManager(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = default;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager& operator=(AssetManager&&) = default;
        ~AssetManager();

        [[nodiscard]] inline const std::filesystem::path& GetRootDirectory() const { return root; };

        // Get asset by snowflake ID
        Asset Get(const AssetID& snowflake);
        std::future<Asset> GetAsync(const AssetID& snowflake);

        // Load an unloaded asset into the asset store
        Asset LoadPath(const std::filesystem::path& fp);
        std::future<Asset> LoadPathAsync(const std::filesystem::path& fp);

    private:
        bool isRunning = true;
        std::filesystem::path root;
        std::unordered_map<AssetID, Asset> assets;
        std::thread fileWatchThread;

        void indexFilesystem(std::filesystem::path dir);
        void fileWatch();
    };

    class AssetNotFoundException : public std::exception
    {
    public:
        AssetNotFoundException(const std::string& what = "") : std::exception(what.c_str()) {}
    };
}
