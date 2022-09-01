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

class AssetManager
{
public:
    static constexpr size_t WATCH_INTERVAL = 1000;

    AssetManager();
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = default;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager& operator=(AssetManager&&) = default;
    ~AssetManager();

    // Load asset by snowflake ID
    Asset Load(const Snowflake& snowflake);
    std::future<Asset> LoadAsync(const Snowflake& snowflake);

    // Load asset directly by filepath
    Asset LoadFile(const std::filesystem::path& fp);
    std::future<Asset> LoadFileAsync(const std::filesystem::path& fp);

    // Converts a raw asset into a binary asset
    static std::filesystem::path ConvertPlainAsset(const std::filesystem::path& fp);

private:
    struct InternalAssetInfo
    {
        std::filesystem::path originalPath;
        std::filesystem::file_time_type lastWriteTime;
    };

    bool isRunning = true;
    std::unordered_map<Snowflake, Asset> assets;
    std::unordered_map<Snowflake, InternalAssetInfo> internalInfo;
    std::thread fileWatchThread;
    std::vector<std::thread> loadThreads;

    void fileWatch();
};

class AssetNotFoundException : public std::exception
{
public:
    AssetNotFoundException(const std::string& what = "") : std::exception(what.c_str()) {}
};