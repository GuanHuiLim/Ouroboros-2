/************************************************************************************//*!
\file           Asset.h
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Aug 30, 2022
\brief          Contains the declaration for the Asset and AssetHeader classes.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <array>
#include <filesystem>

namespace oo
{
    class Asset;
    class AssetManager;

    using AssetID = uint64_t;

    struct AssetMetaContent
    {
        AssetID id;
    };

    struct AssetInfo
    {
        using Callback = std::function<void(void)>;

        std::filesystem::path contentPath;
        std::filesystem::path metaPath;
        std::chrono::steady_clock::time_point timeLoaded = std::chrono::steady_clock::now();
        std::list<Asset*> copies;
        Callback onAssetDestroy = []() {};
        void* data = nullptr;
    };

    class Asset
    {
    public:
        using Extension = const char*;
        template <size_t N>
        using ExtensionList = std::array<Extension, N>;

        static constexpr Extension EXT_META = ".meta";
        static constexpr ExtensionList<3> EXTS_TEXTURE = { ".png", ".jpg", ".jpeg" };
        static constexpr ExtensionList<2> EXTS_FONT = { ".ttf", ".otf" };
        static constexpr ExtensionList<3> EXTS_AUDIO = { ".ogg", ".mp3", ".wav" };

        static AssetID GenerateSnowflake();

        Asset(std::filesystem::path contentPath = {}, AssetID id = GenerateSnowflake());
        Asset(Asset& other);
        Asset(Asset&& other);
        Asset& operator=(Asset& other);
        Asset& operator=(Asset&& other);
        ~Asset();

        [[nodiscard]] inline const AssetID& GetID() const { return id; };
        [[nodiscard]] inline const auto& GetFilePath() const { return info->contentPath; };
        [[nodiscard]] inline const auto& GetMetaFilePath() const { return info->metaPath; };
        [[nodiscard]] inline const auto& GetTimeLoaded() const { return info->timeLoaded; };
        [[nodiscard]] inline const std::list<Asset*>& GetCopies() const { return info->copies; };
        [[nodiscard]] inline size_t GetUseCount() const { return info->copies.size(); };
        [[nodiscard]] inline void* GetRawData() const { return info->data; };
        template<typename T>
        [[nodiscard]] inline T GetData() const { return *reinterpret_cast<T*>(info->data); };

    private:
        AssetID id;
        AssetInfo* info;

        friend AssetManager;
    };

    class AssetDataNotFoundException : public std::exception
    {
    public:
        AssetDataNotFoundException(const std::string& what = "") : std::exception(what.c_str()) {}
    };
}
