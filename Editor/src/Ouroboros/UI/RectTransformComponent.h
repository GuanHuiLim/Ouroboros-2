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

#include "Ouroboros/Geometry/Shapes.h"

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

        bool IsDirty = true;    // set to true for the first frame update!

        bool IsWorldSpace = false;
        
        AnchorPreset Preset = AnchorPreset::AnchorMiddleCentre;
        
        glm::vec3 AnchoredPosition = { 0, 0, 0 };
        glm::vec3 EulerAngles = { 0, 0, 0 };
        glm::vec3 Scale = { 1, 1, 1 };

        glm::vec2 Size = { 100, 100 };
        glm::vec2 Pivot = { 0.5f, 0.5f };
        glm::vec2 AnchorMin = { 0.5f, 0.5f };
        glm::vec2 AnchorMax = { 0.5f, 0.5f };

        glm::vec3 ParentOffset = { 0,0,0 };
        OrientedBoundingBox BoundingVolume = {};

        void SetPreset(AnchorPreset);
        AnchorPreset GetPreset() const;

        void SetAnchoredPosition(glm::vec3);
        void SetEulerAngles(glm::vec3);
        void SetScale(glm::vec3);

        void SetSize(glm::vec2);
        void SetPivot(glm::vec2);
        void SetAnchorMin(glm::vec2);
        void SetAnchorMax(glm::vec2);

        glm::vec3 GetAnchoredPosition() const;
        glm::vec3 GetEulerAngles() const;
        glm::vec3 GetScale() const;

        glm::vec2 GetSize() const;
        glm::vec2 GetPivot() const;
        glm::vec2 GetAnchorMin() const;
        glm::vec2 GetAnchorMax() const;

        RTTR_ENABLE();
    };
}