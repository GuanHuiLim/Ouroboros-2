#pragma once

#include <Ouroboros/Core/Layer.h>
#include <Ouroboros/EventSystem/Event.h>

// Project Tracker related includes
#include <Launcher/Launcher/ProjectTracker.h>

struct ImGuiRestartEvent : public oo::Event
{
};

class EditorLayer final : public oo::Layer
{
private:
    //order matters dont change it
    bool m_demo = true;
    bool m_showDebugInfo = false;

    ProjectTracker m_tracker;
public:
    EditorLayer()
        : oo::Layer{ "EditorLayer" }
    {
        LOG_INFO("Test Info");
        LOG_TRACE("Test Trace");
        LOG_WARN("Test Warn");
        LOG_ERROR("Test Error");
        LOG_CRITICAL("Test Critical");
    }

    virtual void OnAttach() override final;

    // TODO : IMGUI DOESNT WORK YET FOR NOW. VULKAN NEEDS TO BE SET UP
    // PROPERLY FOR IMGUI RENDERING TO TAKE PLACE
    virtual void OnUpdate() override final;

};
