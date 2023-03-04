/************************************************************************************//*!
\file           GameObjectComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes component holding basic information that should be accessible
                by all and all gameobjects should have one of these components by default.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <string>
#include "Utility/UUID.h"
#include <scenegraph/include/scenenode.h>

#include <rttr/type>
#include <Ouroboros/EventSystem/Event.h>
#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
#include "Ouroboros/Asset/Asset.h"
namespace oo
{
    static constexpr std::size_t s_MaxLayerCount = 8;
    using LayerField = std::bitset<s_MaxLayerCount>;
    using LayerMask = LayerField;
    using LayerMatrix = std::unordered_map<LayerField, LayerMask>;


    class GameObjectComponent
    {
    public:
		inline static std::vector<std::string> LayerNames = { "Layer One", "Layer Two","Layer Three","Layer Four","Layer Five","Layer Six","Layer Seven","Layer Eight" };
        
		bool IsPrefab = false;
        bool Active = true;
        bool ActiveInHierarchy = true;
        oo::UUID Id = oo::UUID::Invalid;
        std::string Name = "Default Name Long enough for no short string optimization";
        scenenode::weak_pointer Node = {};

        // Who am I? usually only 1
        LayerField InputLayer{ "00000001" };
        // Who can i collide with?
        LayerField OutputLayer{ "11111111" };

        uint32_t GetInputLayer() const { return InputLayer.to_ulong(); }
        void SetInputLayer(uint32_t inLayer) { InputLayer = inLayer; };

        uint32_t GetOutputLayer() const { return OutputLayer.to_ulong(); }
        void SetOutputLayer(uint32_t outLayer) { OutputLayer = outLayer; };

        struct OnEnableEvent : public Event
        {
            OnEnableEvent(UUID id) : Id{ id } {}

            oo::UUID Id;
        };

        struct OnDisableEvent : public Event
        {
            OnDisableEvent(UUID id) : Id{ id } {}

            oo::UUID Id;
        };

        RTTR_ENABLE();
    };
}