/************************************************************************************//*!
\file           LayerSet.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 05, 2022
\brief          Defines an abstract data type that helps wrap and manage
                Layers and overlay

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "LayerSet.h"

namespace oo
{
    void LayerSet::PushLayer(LayerStack::value_type layer)
    {
        m_addLayerQueue.emplace(layer);
    }

    void LayerSet::PushOverlay(LayerStack::value_type overlay)
    {
        m_addOverlayQueue.emplace(overlay);
    }

    void LayerSet::PopLayer(LayerStack::value_type layer)
    {
        m_removeLayerQueue.emplace(layer);
    }

    void LayerSet::PopOverlay(LayerStack::value_type overlay)
    {
        m_removeOverlayQueue.emplace(overlay);
    }

    void LayerSet::Update()
    {
        Process();
        for (auto& layer : m_layerStack)
        {
            layer->OnUpdate();
        }
    }

    void LayerSet::Process()
    {
        while (!m_removeLayerQueue.empty())
        {
            auto& layer = m_removeLayerQueue.front();
            m_layerStack.PopLayer(layer);
            m_removeLayerQueue.pop();
        }

        while (!m_removeOverlayQueue.empty())
        {
            auto& layer = m_removeOverlayQueue.front();
            m_layerStack.PopOverlay(layer);
            m_removeOverlayQueue.pop();
        }

        while (!m_addLayerQueue.empty())
        {
            auto& layer = m_addLayerQueue.front();
            m_layerStack.PushLayer(layer);
            m_addLayerQueue.pop();
        }

        while (!m_addOverlayQueue.empty())
        {
            auto& layer = m_addOverlayQueue.front();
            m_layerStack.PushOverlay(layer);
            m_addOverlayQueue.pop();
        }
    }
}
