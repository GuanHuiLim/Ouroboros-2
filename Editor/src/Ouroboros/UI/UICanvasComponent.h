/************************************************************************************//*!
\file           UICanvasComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Nov 09, 2022
\brief          Defines a UICanvas Component, which marks GameObjects with a RectTransform
                as the controller for all UI elements parented under it

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <cstdint>

namespace oo
{
    class UICanvasComponent final
    {
    public:
        
        enum class RenderMode : int
        {
            Overlay,
            WorldSpace,
        };

        //bool m_dirty;               // UI elements have changed, need to recalculate
        //bool m_hasChanged;          // UI elements have been recalculated, need to redraw

        RenderMode RenderingMode = RenderMode::Overlay;
        std::uint32_t Layer = 0;
        bool ScaleWithScreenSize = false;

        RTTR_ENABLE();
    };
}