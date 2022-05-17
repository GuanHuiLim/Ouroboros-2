/************************************************************************************//*!
\file           LayerStack.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 24, 2021
\brief          Layerstack holds a vector of layers and provides the interface to 
                interact and control the individual layers. 
                Read layers.h for more explaination on what a layer is.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "LayerStack.h"

namespace oo
{
    LayerStack::~LayerStack()
    {
        for (LayerStack::value_type layer : m_layers)
        {
            layer->OnDetach();
            //delete layer;
        }
    }

    void LayerStack::PushLayer(value_type layer)
    {
        m_layers.emplace(m_layers.begin() + m_layerInsertIndex++, layer);
        layer->OnAttach();
    }

    void LayerStack::PushOverlay(value_type overlay)
    {
        m_layers.emplace_back(overlay);
        overlay->OnAttach();
    }

    void LayerStack::PopLayer(value_type layer)
    {
        auto it = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer);
        if (it != m_layers.begin() + m_layerInsertIndex)
        {
            layer->OnDetach();
            m_layers.erase(it);
            --m_layerInsertIndex;
        }
    }

    void LayerStack::PopOverlay(value_type overlay)
    {
        auto it = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), overlay);
        if (it != m_layers.end())
        {
            overlay->OnDetach();
            m_layers.erase(it);
        }
    }

}