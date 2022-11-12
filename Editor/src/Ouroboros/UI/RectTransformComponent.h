/************************************************************************************//*!
\file           RectTransformComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Nov 09, 2022
\brief          Defines a RectTransform Component, which allows GameObjects to have
                positional and size data which is needed for other UI components like UIImage.
                RectTransform adds additional functionality, mainly the pivot and anchor features,
                to reposition and resize UI elements on the screen

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <rttr/type>
#include <glm/glm.hpp>

namespace oo
{
    class RectTransformComponent final
    {
    public:
        // used mainly for the inspector to easily set common anchor/pivot settings
        enum class AnchorPreset : int
        {
            Custom = 0,

            AnchorTopLeft = 1,
            AnchorTopCentre = 2,
            AnchorTopRight = 3,

            AnchorMiddleLeft = 4,
            AnchorMiddleCentre = 5,
            AnchorMiddleRight = 6,

            AnchorBottomLeft = 7,
            AnchorBottomCentre = 8,
            AnchorBottomRight = 9,

            StretchLeft = 10,
            StretchCentre = 11,
            StretchRight = 12,

            StretchTop = 13,
            StretchMiddle = 14,
            StretchBottom = 15,

            StretchWhole = 16,
        };

        bool IsWorldSpace = false;
        //bool m_dirty = true;
        AnchorPreset Preset = AnchorPreset::AnchorMiddleCentre;
        glm::vec3 AnchoredPosition = { 0,0,0 };
        float Angle = 0.f;
        glm::vec3 Scale = { 1, 1, 1 };
        glm::vec2 Size = { 100, 100 };
        glm::vec2 Pivot = { 0.5f, 0.5f };
        glm::vec2 AnchorMin = { 0.5f, 0.5f };
        glm::vec2 AnchorMax = { 0.5f, 0.5f };
        
        RTTR_ENABLE();
    };
}