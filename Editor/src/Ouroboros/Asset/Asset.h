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

class Asset;
class AssetManager;

using Snowflake = uint64_t;

struct AssetHeader
{
    static constexpr size_t EXT_SIZE = 8;

    Snowflake id;
    char extOriginal[EXT_SIZE];

    AssetHeader(const std::string& ext = "");

    static Snowflake GenerateSnowflake();
};

struct AssetMeta
{
    std::chrono::steady_clock::time_point timeLoaded = std::chrono::steady_clock::now();
    std::list<Asset*> copies;
    void* data = nullptr;
};

class Asset
{
public:
    using Extension = const char*;
    template <size_t N>
    using ExtensionList = std::array<Extension, N>;

    static constexpr Extension EXT_DATA_GENERIC = ".data";
    static constexpr ExtensionList<3> EXTS_TEXTURE = { ".png", ".jpg", ".jpeg" };
    static constexpr ExtensionList<2> EXTS_FONT = { ".ttf", ".otf" };
    static constexpr ExtensionList<3> EXTS_AUDIO = { ".ogg", ".mp3", ".wav" };

    Asset();
    Asset(Asset& other);
    Asset(Asset&& other);
    Asset& operator=(Asset& other);
    Asset& operator=(Asset&& other);
    ~Asset();

    [[nodiscard]] inline const AssetHeader& GetHeader() const { return *header; };
    [[nodiscard]] inline const auto& GetTimeLoaded() const { return meta->timeLoaded; };
    [[nodiscard]] inline const std::list<Asset*>& GetCopies() const { return meta->copies; };
    [[nodiscard]] inline size_t GetUseCount() const { return meta->copies.size(); };
    [[nodiscard]] inline void* GetRawData() const { return meta->data; };

    template<typename T>
    T* GetData() const;

private:
    AssetHeader* header;
    AssetMeta* meta;

    friend AssetManager;
};

class AssetDataNotFoundException : public std::exception
{
public:
    AssetDataNotFoundException(const std::string& what = "") : std::exception(what.c_str()) {}
};

template<typename T>
inline T* Asset::GetData() const
{
    if (meta->data)
    {
        return reinterpret_cast<T*>(meta->data);
    }
    throw AssetDataNotFoundException();
}
