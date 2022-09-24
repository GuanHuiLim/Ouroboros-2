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

struct gfxModel;

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
        enum class Type
        {
            Text = 0,
            Texture,
            Font,
            Audio,
            Model,
        };

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Retrieves the data stored by the asset of a given type.
        /// </summary>
        /// <typeparam name="T">The type of data.</typeparam>
        /// <returns>The data.</returns>
        template<typename T>
        [[nodiscard]] inline T GetData() const;

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
        Type type = Type::Text;
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

        static constexpr AssetID ID_NULL = 0;
        static constexpr Extension EXT_META = ".meta";
        static constexpr ExtensionList<4> EXTS_TEXTURE = { ".png", ".jpg", ".jpeg", ".dds" };
        static constexpr ExtensionList<2> EXTS_FONT = { ".ttf", ".otf" };
        static constexpr ExtensionList<3> EXTS_AUDIO = { ".ogg", ".mp3", ".wav" };
        static constexpr ExtensionList<1> EXTS_MODEL = { ".fbx" };

        /* --------------------------------------------------------------------------- */
        /* Static Functions                                                            */
        /* --------------------------------------------------------------------------- */

        static AssetID GenerateSnowflake();

        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* --------------------------------------------------------------------------- */

        Asset(std::filesystem::path contentPath = {}, AssetID id = ID_NULL);
        Asset(const Asset& other);
        Asset(Asset&& other);
        Asset& operator=(const Asset& other);
        Asset& operator=(Asset&& other);
        ~Asset();

        /* --------------------------------------------------------------------------- */
        /* Getters                                                                     */
        /* --------------------------------------------------------------------------- */

        [[nodiscard]] inline bool IsValid() const { return id == ID_NULL; };
        [[nodiscard]] inline const AssetID& GetID() const { return id; };
        [[nodiscard]] inline const auto& GetFilePath() const { return info->contentPath; };
        [[nodiscard]] inline const auto& GetMetaFilePath() const { return info->metaPath; };
        [[nodiscard]] inline const auto& GetTimeLoaded() const { return info->timeLoaded; };
        [[nodiscard]] inline const std::list<Asset*>& GetCopies() const { return info->copies; };
        [[nodiscard]] inline size_t GetUseCount() const { return info->copies.size(); };
        [[nodiscard]] inline void* GetRawData() const { return info->data; };
        [[nodiscard]] inline bool HasData() const { return info->data; };
        [[nodiscard]] inline AssetInfo::Type GetType() const { return info->type; }

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Retrieves the data stored by the asset of a given type.
        /// </summary>
        /// <typeparam name="T">The type of data.</typeparam>
        /// <returns>The data.</returns>
        template<typename T>
        [[nodiscard]] inline T GetData() const;

        /// <summary>
        /// Retrieves the types explicitly stored by the asset.
        /// </summary>
        /// <returns>The type indices.</returns>
        std::vector<std::type_index> GetBespokeTypes() const;

        /* --------------------------------------------------------------------------- */
        /* Bespoke Functions                                                           */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Retrieves the submodel from the asset.
        /// Throws an exception if the asset is not a model.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <returns>The submodel.</returns>
        gfxModel& GetSubmodel(size_t index = 0);

        /// <summary>
        /// Retrieves number of submodel in the asset.
        /// Throws an exception if the asset is not a model.
        /// </summary>
        /// <returns>The number.</returns>
        size_t GetSubmodelCount() const;


        // TODO: write more wrappers for accessing data eg submeshes, imguiID


    private:
        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Shorthand for calling the create callback of the asset's info.
        /// </summary>
        void createData();

        /// <summary>
        /// Shorthand for calling the destroy callback of the asset's info.
        /// </summary>
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
    inline T AssetInfo::GetData() const
    {
        // Segmented structure reinterpretation
        if (dataTypeOffsets.find(std::type_index(typeid(T))) != dataTypeOffsets.end())
        {
            size_t offset = dataTypeOffsets.at(std::type_index(typeid(T)));
            char* ptr = reinterpret_cast<char*>(data);
            return *reinterpret_cast<T*>(ptr + offset);
        }

        // Direct reinterpretation
        return *reinterpret_cast<T*>(data);
    }

    template<typename T>
    inline T Asset::GetData() const
    {
        return info->GetData<T>();
    }

    class AssetDataNotFoundException : public std::exception
    {
    public:
        AssetDataNotFoundException(const std::string& what = "Asset Data Not Found") : std::exception(what.c_str()) {}
    };

    class AssetInvalidTypeException : public std::exception
    {
    public:
        AssetInvalidTypeException(const std::string& what = "Asset Invalid Type") : std::exception(what.c_str()) {}
    };
}
