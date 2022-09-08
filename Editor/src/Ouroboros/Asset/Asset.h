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
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

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
        /* --------------------------------------------------------------------------- */
        /* Type Definitions                                                            */
        /* --------------------------------------------------------------------------- */

        using Callback = std::function<void(AssetInfo&)>;
        using DataTypeOffsetsType = std::unordered_map<std::type_index, size_t>;

        /* --------------------------------------------------------------------------- */
        /* Members                                                                     */
        /* --------------------------------------------------------------------------- */

        std::filesystem::path contentPath;
        std::filesystem::path metaPath;
        std::chrono::file_clock::time_point timeLoaded = std::chrono::file_clock::now();
        std::list<Asset*> copies;
        Callback onAssetCreate = [](AssetInfo&) {};
        Callback onAssetDestroy = [](AssetInfo&) {};
        void* data = nullptr;
        DataTypeOffsetsType dataTypeOffsets;
    };

    class Asset
    {
    public:
        /* --------------------------------------------------------------------------- */
        /* Type Definitions                                                            */
        /* --------------------------------------------------------------------------- */

        using Extension = const char*;
        template <size_t N>
        using ExtensionList = std::array<Extension, N>;

        /* --------------------------------------------------------------------------- */
        /* Constants                                                                   */
        /* --------------------------------------------------------------------------- */

        static constexpr Extension EXT_META = ".meta";
        static constexpr ExtensionList<3> EXTS_TEXTURE = { ".png", ".jpg", ".jpeg" };
        static constexpr ExtensionList<2> EXTS_FONT = { ".ttf", ".otf" };
        static constexpr ExtensionList<3> EXTS_AUDIO = { ".ogg", ".mp3", ".wav" };

        /* --------------------------------------------------------------------------- */
        /* Static Functions                                                            */
        /* --------------------------------------------------------------------------- */

        static AssetID GenerateSnowflake();

        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* --------------------------------------------------------------------------- */

        Asset(std::filesystem::path contentPath = {}, AssetID id = GenerateSnowflake());
        Asset(Asset& other);
        Asset(Asset&& other);
        Asset& operator=(Asset& other);
        Asset& operator=(Asset&& other);
        ~Asset();

        /* --------------------------------------------------------------------------- */
        /* Getters                                                                     */
        /* --------------------------------------------------------------------------- */

        [[nodiscard]] inline const AssetID& GetID() const { return id; };
        [[nodiscard]] inline const auto& GetFilePath() const { return info->contentPath; };
        [[nodiscard]] inline const auto& GetMetaFilePath() const { return info->metaPath; };
        [[nodiscard]] inline const auto& GetTimeLoaded() const { return info->timeLoaded; };
        [[nodiscard]] inline const std::list<Asset*>& GetCopies() const { return info->copies; };
        [[nodiscard]] inline size_t GetUseCount() const { return info->copies.size(); };
        [[nodiscard]] inline void* GetRawData() const { return info->data; };
        [[nodiscard]] inline bool HasData() const { return info->data; };
        template<typename T>
        [[nodiscard]] inline T GetData() const;

    private:
        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /****************************************************************************//*!
        @brief  Shorthand for calling the create callback of the asset's info.
        *//*****************************************************************************/
        void createData();

        /****************************************************************************//*!
        @brief  Shorthand for calling the destroy callback of the asset's info.
        *//*****************************************************************************/
        void destroyData();

        /* --------------------------------------------------------------------------- */
        /* Members                                                                     */
        /* --------------------------------------------------------------------------- */

        AssetID id;
        AssetInfo* info;

        /* --------------------------------------------------------------------------- */
        /* Friends                                                                     */
        /* --------------------------------------------------------------------------- */

        friend AssetManager;
    };

    template<typename T>
    inline T Asset::GetData() const
    {
        // Segmented structure reinterpretation
        if (info->dataTypeOffsets.find(std::type_index(typeid(T))) != info->dataTypeOffsets.end())
        {
            size_t offset = info->dataTypeOffsets[std::type_index(typeid(T))];
            char* ptr = reinterpret_cast<char*>(info->data);
            return *reinterpret_cast<T*>(ptr + offset);
        }

        // Direct reinterpretation
        return *reinterpret_cast<T*>(info->data);
    }

    class AssetDataNotFoundException : public std::exception
    {
    public:
        AssetDataNotFoundException(const std::string& what = "") : std::exception(what.c_str()) {}
    };
}
