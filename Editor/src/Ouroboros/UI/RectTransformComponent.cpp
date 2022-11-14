/************************************************************************************//*!
\file           RectTransformComponent.cpp
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
#include "pch.h"
#include "RectTransformComponent.h"
#include <rttr/registration.h>
#include "App/Editor/Properties/UI_metadata.h"
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::enumeration<RectTransformComponent::AnchorPreset>("Anchor Preset")
        (
            value("Custom", RectTransformComponent::AnchorPreset::Custom),

            value("Top Left", RectTransformComponent::AnchorPreset::AnchorTopLeft),
            value("Top Centre", RectTransformComponent::AnchorPreset::AnchorTopCentre),
            value("Top Right", RectTransformComponent::AnchorPreset::AnchorTopRight),

            value("Middle Left", RectTransformComponent::AnchorPreset::AnchorMiddleLeft),
            value("Middle Centre", RectTransformComponent::AnchorPreset::AnchorMiddleCentre),
            value("Middle Right", RectTransformComponent::AnchorPreset::AnchorMiddleRight),

            value("Bottom Left", RectTransformComponent::AnchorPreset::AnchorBottomLeft),
            value("Bottom Centre", RectTransformComponent::AnchorPreset::AnchorBottomCentre),
            value("Bottom Right", RectTransformComponent::AnchorPreset::AnchorBottomRight),

            value("Stretch Left", RectTransformComponent::AnchorPreset::StretchLeft),
            value("Stretch Centre", RectTransformComponent::AnchorPreset::StretchCentre),
            value("Stretch Right", RectTransformComponent::AnchorPreset::StretchRight),

            value("Stretch Top", RectTransformComponent::AnchorPreset::StretchTop),
            value("Stretch Middle", RectTransformComponent::AnchorPreset::StretchMiddle),
            value("Stretch Bottom", RectTransformComponent::AnchorPreset::StretchBottom),

            value("Stretch Whole", RectTransformComponent::AnchorPreset::StretchWhole)
        );

        registration::class_<RectTransformComponent>("Rect Transform")
            .property("Preset", &RectTransformComponent::GetPreset, &RectTransformComponent::SetPreset)
            .property("Anchored Position", &RectTransformComponent::GetAnchoredPosition, &RectTransformComponent::SetAnchoredPosition)
            .property("Rotation Angles", &RectTransformComponent::GetEulerAngles, &RectTransformComponent::SetEulerAngles)
            .property("Scaling", &RectTransformComponent::GetScale, & RectTransformComponent::SetScale)
            .property("Size", &RectTransformComponent::GetSize, &RectTransformComponent::SetSize)
            .property("Pivot", &RectTransformComponent::GetPivot, &RectTransformComponent::SetPivot)(metadata(UI_metadata::DRAG_SPEED, 0.01f))
            .property("Anchor Min", &RectTransformComponent::GetAnchorMin, &RectTransformComponent::SetAnchorMin)(metadata(UI_metadata::DRAG_SPEED, 0.01f))
            .property("Anchor Max", &RectTransformComponent::GetAnchorMax, &RectTransformComponent::SetAnchorMax)(metadata(UI_metadata::DRAG_SPEED, 0.01f))
            .property_readonly("Parent Offset", &RectTransformComponent::ParentOffset)
            .property_readonly("IsDirty", &RectTransformComponent::IsDirty)
            .property_readonly("Bounding Volume", &RectTransformComponent::BoundingVolume)
            ;
    }

    void oo::RectTransformComponent::SetPreset(AnchorPreset new_preset)
    {
        if (Preset == AnchorPreset::Custom || Preset == new_preset)
            return;
        
        IsDirty = true;
        Preset = new_preset;
        //TODO do a bit more default settings here perhaps.
        switch (Preset)
        {
        case AnchorPreset::AnchorTopLeft:
            AnchorMin = AnchorMax = glm::vec2{ 0, 1 };
            Pivot = glm::vec2{ 0.0f, 1.0f };
            break;
        case AnchorPreset::AnchorTopCentre:
            AnchorMin = AnchorMax = glm::vec2{ 0.5f, 1 };
            Pivot = glm::vec2{ 0.5f, 1.0f };
            break;
        case AnchorPreset::AnchorTopRight:
            AnchorMin = AnchorMax = glm::vec2{ 1, 1 };
            Pivot = glm::vec2{ 1.0f, 1.0f };
            break;

        case AnchorPreset::AnchorMiddleLeft:
            AnchorMin = AnchorMax = glm::vec2{ 0, 0.5f };
            Pivot = glm::vec2{ 0.0f, 0.5f };
            break;
        case AnchorPreset::AnchorMiddleCentre:
            AnchorMin = AnchorMax = glm::vec2{ 0.5f, 0.5f };
            Pivot = glm::vec2{ 0.5f, 0.5f };
            break;
        case AnchorPreset::AnchorMiddleRight:
            AnchorMin = AnchorMax = glm::vec2{ 1, 0.5f };
            Pivot = glm::vec2{ 1.0f, 0.5f };
            break;

        case AnchorPreset::AnchorBottomLeft:
            AnchorMin = AnchorMax = glm::vec2{ 0, 0 };
            Pivot = glm::vec2{ 0.0f, 0.0f };
            break;
        case AnchorPreset::AnchorBottomCentre:
            AnchorMin = AnchorMax = glm::vec2{ 0.5f, 0 };
            Pivot = glm::vec2{ 0.5f, 0.0f };
            break;
        case AnchorPreset::AnchorBottomRight:
            AnchorMin = AnchorMax = glm::vec2{ 1, 0 };
            Pivot = glm::vec2{ 1.0f, 0.0f };
            break;

        case AnchorPreset::StretchLeft:
            AnchorMin = glm::vec2{ 0, 0 };
            AnchorMax = glm::vec2{ 0, 1 };
            Pivot = glm::vec2{ 0, 0.5f };
            break;
        case AnchorPreset::StretchCentre:
            AnchorMin = glm::vec2{ 0.5f, 0 };
            AnchorMax = glm::vec2{ 0.5f, 1 };
            Pivot = glm::vec2{ 0.5f, 0.5f };
            break;
        case AnchorPreset::StretchRight:
            AnchorMin = glm::vec2{ 1, 0 };
            AnchorMax = glm::vec2{ 1, 1 };
            Pivot = glm::vec2{ 1, 0.5f };
            break;

        case AnchorPreset::StretchTop:
            AnchorMin = glm::vec2{ 0, 1 };
            AnchorMax = glm::vec2{ 1, 1 };
            Pivot = glm::vec2{ 0.5f, 1 };
            break;
        case AnchorPreset::StretchMiddle:
            AnchorMin = glm::vec2{ 0, 0.5f };
            AnchorMax = glm::vec2{ 1, 0.5f };
            Pivot = glm::vec2{ 0.5f, 0.5f };
            break;
        case AnchorPreset::StretchBottom:
            AnchorMin = glm::vec2{ 0, 0 };
            AnchorMax = glm::vec2{ 1, 0 };
            Pivot = glm::vec2{ 0.5f, 0 };
            break;

        case AnchorPreset::StretchWhole:
            AnchorMin = glm::vec2{ 0, 0 };
            AnchorMax = glm::vec2{ 1, 1 };
            Pivot = glm::vec2{ 0.5f, 0.5f };
            break;
        }
    }

    RectTransformComponent::AnchorPreset oo::RectTransformComponent::GetPreset() const
    {
        return Preset;
    }

    void RectTransformComponent::SetAnchoredPosition(glm::vec3 new_anchor)
    {
        IsDirty = true;
        AnchoredPosition = new_anchor;
    }
    void RectTransformComponent::SetEulerAngles(glm::vec3 new_euler)
    {
        IsDirty = true;
        EulerAngles = new_euler;
    }
    void RectTransformComponent::SetScale(glm::vec3 new_scale)
    {
        IsDirty = true;
        Scale = new_scale;
    }
    void RectTransformComponent::SetSize(glm::vec2 new_size)
    {
        IsDirty = true;
        Size = new_size;
    }
    void RectTransformComponent::SetPivot(glm::vec2 new_pivot)
    {
        IsDirty = true;
        Pivot = new_pivot;
    }
    void RectTransformComponent::SetAnchorMin(glm::vec2 new_min)
    {
        IsDirty = true;
        AnchorMin = new_min;
    }
    void RectTransformComponent::SetAnchorMax(glm::vec2 new_max)
    {
        IsDirty = true;
        AnchorMax = new_max;
    }
    glm::vec3 RectTransformComponent::GetAnchoredPosition() const
    {
        return AnchoredPosition;
    }
    glm::vec3 RectTransformComponent::GetEulerAngles() const
    {
        return EulerAngles;
    }
    glm::vec3 RectTransformComponent::GetScale() const
    {
        return Scale;
    }
    glm::vec2 RectTransformComponent::GetSize() const
    {
        return Size;
    }
    glm::vec2 RectTransformComponent::GetPivot() const
    {
        return Pivot;
    }
    glm::vec2 RectTransformComponent::GetAnchorMin() const
    {
        return AnchorMin;
    }
    glm::vec2 RectTransformComponent::GetAnchorMax() const
    {
        return AnchorMax;
    }
}