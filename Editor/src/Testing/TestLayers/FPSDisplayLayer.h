/************************************************************************************//*!
\file           FPSDisplayLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2022
\brief          Debugging Layer that is used for debugging builds

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once


#include <Ouroboros/Core/Base.h>
#include <Ouroboros/Core/Layer.h>
#include <Ouroboros/Core/Timer.h>
#include <imgui/imgui.h>

class FPSDisplayLayer final : public oo::Layer
{
private:
    
public:
    FPSDisplayLayer()
        : Layer{ "FPSDebug Layer" }
    {
        LOG_TRACE("SUCCESSFULLY LOADED FPS DEBUG LAYER");
    }

    void OnAttach() override final
    {
    }

    void OnDetach() override final
    {
    }

    void OnUpdate() override final
    {
        oo::timer::TimeDebugInfo timeDebugInfo = oo::timer::get_cumulated_debug_info();

        ImGui::Begin("fpsviewer", nullptr,
            ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS                %.2f", timeDebugInfo.AvgFPS);
        ImGui::Text("DeltaTime          %.6f", timeDebugInfo.AvgDeltaTime);
        ImGui::Text("Unscaled FPS       %.2f", timeDebugInfo.AvgUnscaledFPS);
        ImGui::Text("Unscaled DeltaTime %.6f", timeDebugInfo.AvgUnscaledDeltaTime);
        ImGui::Text("Raw FPS            %.2f", timeDebugInfo.AvgRawFPS);
        ImGui::Text("Raw DeltaTime      %.6f", timeDebugInfo.AvgRawDeltaTime);
        ImGui::Text("Timescale          %.2f", timeDebugInfo.CurrentTimeScale);
        ImGui::Text("Time elpased       %.2f", timeDebugInfo.TimeElapsed);
        ImGui::End();
    }
};
