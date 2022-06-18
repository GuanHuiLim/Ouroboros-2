/************************************************************************************//*!
\file           LayerSet.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 05, 2022
\brief          Declares an abstract data type that helps wrap and manage
                Layers and overlay

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "LayerStack.h"
#include <queue>

namespace oo
{
    class LayerSet final
    {
    public:
        void PushLayer(LayerStack::value_type layer);
        void PushOverlay(LayerStack::value_type overlay);

        void PopLayer(LayerStack::value_type layer);
        void PopOverlay(LayerStack::value_type overlay);

        void Update();
    private:
        void Process();

        LayerStack m_layerStack;
        std::queue<LayerStack::value_type> m_addLayerQueue;
        std::queue<LayerStack::value_type> m_addOverlayQueue;
        std::queue<LayerStack::value_type> m_removeLayerQueue;
        std::queue<LayerStack::value_type> m_removeOverlayQueue;
    };
}