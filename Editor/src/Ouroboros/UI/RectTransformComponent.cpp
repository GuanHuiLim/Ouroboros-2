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
            .property("Preset", &RectTransformComponent::Preset)
            .property("Anchored Position", &RectTransformComponent::AnchoredPosition)
            .property("Rotation Angle", &RectTransformComponent::Angle)
            .property("Scaling", &RectTransformComponent::Scale)
            .property("Size", &RectTransformComponent::Size)
            .property("Pivot", &RectTransformComponent::Pivot)(metadata(UI_metadata::DRAG_SPEED, 0.01f))
            .property("Anchor Min", &RectTransformComponent::AnchorMin)(metadata(UI_metadata::DRAG_SPEED, 0.01f))
            .property("Anchor Max", &RectTransformComponent::AnchorMax)(metadata(UI_metadata::DRAG_SPEED, 0.01f))
            ;
    }
}