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
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

#include <imgui/imgui.h>
#include <rttr/variant.h>

#include "OO_Vulkan/src/MeshModel.h"
#include "Ouroboros/Audio/Audio.h"

namespace oo
{
    class Asset;
    class AssetManager;

    using Snowflake = uint64_t;
    using AssetID = Snowflake;

    /// <summary>
    /// Content for a meta file.
    /// </summary>
    struct AssetMetaContent
    {
        AssetID id;
    };

    /// <summary>
    /// Underlying data for an asset.
    /// </summary>
    struct AssetInfo
    {
        /* --------------------------------------------------------------------------- */
        /* Type Definitions                                                            */
        /* --------------------------------------------------------------------------- */

        using Callback = std::function<void(AssetInfo&)>;
        enum class Type
        {
            Text = 0,
            Texture,
            Font,
            Audio,
            Model,
        };

        /* --------------------------------------------------------------------------- */
        /* Constants                                                                   */
        /* --------------------------------------------------------------------------- */

        static constexpr AssetID ID_NULL = 0;

        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* --------------------------------------------------------------------------- */

        ~AssetInfo();

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Generates a unique Snowflake ID.
        /// </summary>
        /// <returns>The Snowflake ID.</returns>
        static Snowflake GenerateSnowflake();

        /// <summary>
        /// Reloads the data from the file into the asset.
        /// </summary>
        void Reload();

        /// <summary>
        /// Reloads the data from the file into the asset.
        /// </summary>
        /// <param name="type">The explicit type of asset to load as.</param>
        void Reload(AssetInfo::Type type);

        /// <summary>
        /// Unloads the data in the asset.
        /// </summary>
        void Unload();

        /// <summary>
        /// Writes the data from the asset into the file.
        /// </summary>
        void Overwrite();

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

        AssetID id;
        std::filesystem::path contentPath;
        std::filesystem::path metaPath;
        std::chrono::file_clock::time_point timeLoaded = std::chrono::file_clock::now();
        Callback onAssetCreate = [](AssetInfo&) {};
        Callback onAssetDestroy = [](AssetInfo&) {};
        std::vector<rttr::variant> data;
        Type type = Type::Text;
        bool isDataLoaded = false;
    };

    /// <summary>
    /// Interface for an AssetInfo.
    /// </summary>
    class Asset
    {
    public:
        /* --------------------------------------------------------------------------- */
        /* Type Definitions                                                            */
        /* --------------------------------------------------------------------------- */

        using Extension = const char*;
        template <size_t N>
        using ExtensionList = std::array<Extension, N>;
        using AssetInfoPtr = std::weak_ptr<AssetInfo>;

        /* --------------------------------------------------------------------------- */
        /* Constants                                                                   */
        /* --------------------------------------------------------------------------- */

        static constexpr AssetID ID_NULL = AssetInfo::ID_NULL;
        static constexpr Extension EXT_META = ".meta";
        static constexpr ExtensionList<4> EXTS_TEXTURE = { ".png", ".jpg", ".jpeg", ".dds" };
        static constexpr ExtensionList<2> EXTS_FONT = { ".ttf", ".otf" };
        static constexpr ExtensionList<3> EXTS_AUDIO = { ".ogg", ".mp3", ".wav" };
        static constexpr ExtensionList<1> EXTS_MODEL = { ".fbx" };

        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* --------------------------------------------------------------------------- */

        Asset() = default;

        /* --------------------------------------------------------------------------- */
        /* Getters                                                                     */
        /* --------------------------------------------------------------------------- */

#define AI_TYPE(_PROP) decltype(AssetInfo::_PROP)
#define AI_VALUE_OR_DEFAULT(_PROP) if (auto sp = info.lock()) return sp->_PROP; return {};
#define AI_GETTER(_NAME, _PROP) AI_TYPE(_PROP) _NAME() const { AI_VALUE_OR_DEFAULT(_PROP) }

        [[nodiscard]] inline bool IsValid() const { return !info.expired(); };
        [[nodiscard]] inline AI_GETTER(HasData, data.size() > 0);
        [[nodiscard]] inline AI_GETTER(GetID, id);
        [[nodiscard]] inline AI_GETTER(GetFilePath, contentPath);
        [[nodiscard]] inline AI_GETTER(GetMetaFilePath, metaPath);
        [[nodiscard]] inline AI_GETTER(GetTimeLoaded, timeLoaded);
        [[nodiscard]] inline AI_GETTER(GetRawData, data);
        [[nodiscard]] inline AI_GETTER(GetType, type);
        [[nodiscard]] inline AI_GETTER(IsDataLoaded, isDataLoaded);

#undef AI_GETTER
#undef AI_VALUE_OR_DEFAULT
#undef AI_TYPE

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Generates a unique Snowflake ID.
        /// </summary>
        /// <returns>The Snowflake ID.</returns>
        static Snowflake GenerateSnowflake();

        /// <summary>
        /// Reloads the data from the file into the asset.
        /// </summary>
        void Reload();

        /// <summary>
        /// Reloads the data from the file into the asset.
        /// </summary>
        /// <param name="type">The explicit type of asset to load as.</param>
        void Reload(AssetInfo::Type type);

        /// <summary>
        /// Unloads the data in the asset.
        /// </summary>
        void Unload();

        /// <summary>
        /// Writes the data from the asset into the file.
        /// </summary>
        void Overwrite();

        /// <summary>
        /// Retrieves the data stored by the asset of a given type.
        /// </summary>
        /// <typeparam name="T">The type of data.</typeparam>
        /// <returns>The data.</returns>
        template<typename T>
        [[nodiscard]] inline T GetData() const;

        /* --------------------------------------------------------------------------- */
        /* Bespoke Functions                                                           */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Retrieves number of submodel in the asset.
        /// Throws an exception if the asset is not a model.
        /// </summary>
        /// <returns>The number.</returns>
        size_t GetSubmodelCount() const;


        // TODO: write more wrappers for accessing data eg submeshes, imguiID


    private:
        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* --------------------------------------------------------------------------- */

        Asset(AssetInfoPtr info) : info{ info } {}

        /* --------------------------------------------------------------------------- */
        /* Members                                                                     */
        /* --------------------------------------------------------------------------- */

        AssetInfoPtr info;

        /* --------------------------------------------------------------------------- */
        /* Friends                                                                     */
        /* --------------------------------------------------------------------------- */

        friend AssetManager;
    };

    template<typename T>
    inline T AssetInfo::GetData() const
    {
        if (!isDataLoaded)
            Reload();
        for (auto& d : data)
        {
            if constexpr (std::is_pointer<T>::value)
            {
                if (d.is_type<T>())
                    return d.get_value<T>();
                using T_sptr = std::shared_ptr<std::remove_pointer_t<T>>;
                if (d.is_type<T_sptr>())
                    return d.get_value<T_sptr>().get();
            }
            else
            {
                if (d.is_type<T>())
                    return d.get_value<T>();
            }
        }
        return {};
    }

    template<typename T>
    inline T Asset::GetData() const
    {
        if (auto sp = info.lock())
            return sp->GetData<T>();
        return {};
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
