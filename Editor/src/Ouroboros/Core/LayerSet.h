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

        void Process();

    private:
        LayerStack m_layerStack;
        std::queue<LayerStack::value_type> m_addLayerQueue;
        std::queue<LayerStack::value_type> m_addOverlayQueue;
        std::queue<LayerStack::value_type> m_removeLayerQueue;
        std::queue<LayerStack::value_type> m_removeOverlayQueue;
    };
}