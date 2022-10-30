/************************************************************************************//*!
\file          AnimatorTimelineView.cpp
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 22, 2022
\brief         File Contains the definition needed to create an Animator Timeline View
               for the engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "AnimationTimelineView.h"
#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/Scene/Scene.h>
#include "App/Editor/UI/Object Editor/Hierarchy.h"
#include "App/Editor/Utility/ImGuiManager.h"

constexpr ImGuiID popUpOptionTimeline = 700;
int AnimationTimelineView::currentKeyFrame;
float AnimationTimelineView::unitPerFrame = 0.1f;
float AnimationTimelineView::currentTime = 0.0f;

void AnimationTimelineView::Show()
{
    auto& selected_items = Hierarchy::GetSelected();    //temporary fix, ideally wanna get from animatorcontroller file
    auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();

    if (selected_items.size() <= 0)
        return;

    auto gameObject = scene->FindWithInstanceID(*selected_items.begin());

    if (gameObject->HasComponent<oo::AnimationComponent>() == false)
        return;

    animator = &gameObject.get()->GetComponent<oo::AnimationComponent>();

    DisplayAnimationTimeline(animator);
}

void AnimationTimelineView::DisplayAnimationTimeline(oo::AnimationComponent* _animator)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Current:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(48.0f);
    ImGui::DragInt("##currentFrame", &currentKeyFrame, 1.0f, 0, 1000);
    ImGui::SameLine();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(48.0f);
    ImGui::Text("Unit Per Frame:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(48.0f);
    ImGui::DragFloat("##unitperframe", &unitPerFrame, 0.01f, 0.0f, 1000.0f, "%.2f");
    ImGui::SameLine();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(48.0f);
    ImGui::Text("Current Time:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(48.0f);
    currentTime = currentKeyFrame * unitPerFrame;
    ImGui::InputFloat("##currentTime", &currentTime, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();

    ImGui::NewLine();

    ImGui::BeginChildFrame(1, ImVec2(0, 0));
    {
        ImGuiStyle& style = ImGui::GetStyle();
        //ImVec2 toolBarSize = DrawToolbar(_animator, OnToolbarPressed);

        static bool open = false;
        static std::string animName = "empty";
        auto& temp = _animator->GetActualComponent().animTree->groups;

        //Do Toolbar here

        //To Get Which Node to Modify, Temporary Solution
        ImGui::PushItemWidth(160);
        ImGui::BeginGroup();
        ImGui::InputText("##animationstuff", &animName, ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        if (ImGui::ArrowButton("downbtn", ImGuiDir_Down))
        {
            open = !open;
        }
        if (open)
        {
            if (ImGui::BeginListBox("##animation"))
            {
                for (auto it = temp.begin(); it != temp.end(); ++it)
                {
                    for (auto it2 = it->second.nodes.begin(); it2 != it->second.nodes.end(); ++it2)
                    {
                        if (ImGui::Selectable(it2->second.name.c_str()))
                        {
                            animName = it2->second.name;
                            animation = &it2->second.GetAnimation();
                            currentKeyFrame = 0;
                            open = !open;
                        }
                    }
                }
            }
            ImGui::EndListBox();
        }
        ImGui::EndGroup();
        ImGui::PopItemWidth();

        ImGui::SameLine();

        DrawTimeLine(animation, style.ItemSpacing.y, ImGui::GetItemRectSize().y);
    }
    ImGui::EndChildFrame();
}

void AnimationTimelineView::DrawTimeLine(oo::Anim::Animation* _animation, float headerYPadding, float headerHeight)
{
    auto drawList = ImGui::GetWindowDrawList();
    auto style = ImGui::GetStyle();
    auto contentRegion = ImGui::GetContentRegionAvail();
    auto headerSize = ImVec2(0, 0);
    headerSize.x = contentRegion.x - (style.ScrollbarSize);
    headerSize.y = headerHeight + headerYPadding;

    visibleEndingFrame = GetFrameFromTimelinePos(headerSize.x);

    timelineRegionMin = ImGui::GetCursorScreenPos();
    timelineRegionMax = ImVec2(timelineRegionMin.x + headerSize.x, timelineRegionMin.y + headerSize.y);
    timelineRegionMax.y = timelineRegionMin.y + contentRegion.y;

    ImGui::PushClipRect(timelineRegionMin, timelineRegionMax, false);
    {
        ImGui::InvisibleButton("##header-region", headerSize);

        //set frame
        if (ImGui::IsItemHovered())
        {
            auto hoveringFrame = GetFrameFromTimelinePos(ImGui::GetMousePos().x - timelineRegionMin.x) * unitPerFrame;
            ImGui::BeginTooltip();
            size_t decimalPointPos = std::to_string(hoveringFrame).find_first_of(".");
            std::string hoverFrameText = std::to_string(hoveringFrame).substr(0, decimalPointPos + 3);
            ImGui::Text(hoverFrameText.c_str());
            ImGui::EndTooltip();

            if (ImGui::IsMouseDown(0))
                currentKeyFrame = hoveringFrame;
        }

        //panning timeline
        if(ImGui::IsMouseHoveringRect(timelineRegionMin, timelineRegionMax, false))
        {
            if (ImGui::IsMouseDragging(1, 0))
            {
                accumulatedPanningDeltaX += ImGui::GetIO().MouseDelta.x;

                if (!ImGui::IsWindowFocused())
                    ImGui::SetWindowFocus();

                int framesToMove = (int)std::floor(accumulatedPanningDeltaX / pixelsPerFrame);
                if (framesToMove != 0)
                {
                    isPanningTimeline = true;
                    accumulatedPanningDeltaX -= framesToMove * pixelsPerFrame;
                    visibleStartingFrame -= framesToMove;
                }
            }
            else
            {
                isPanningTimeline = false;
                accumulatedPanningDeltaX = 0.0f;
            }
        }

        //draw all timeline lines
        int frames = visibleEndingFrame - visibleStartingFrame;
        for (int f = 0; f < frames; ++f)
        {
            int frame = f + visibleStartingFrame;
            auto lineStart = timelineRegionMin;
            lineStart.x += lineStartOffset + f * pixelsPerFrame;
            auto lineEnd = ImVec2(lineStart.x, lineStart.y + headerSize.y);

            if (frame % majorLinePerLines == 0)
            {
                std::string numberString = std::to_string(frame * unitPerFrame);
                size_t decimalPos = numberString.find_first_of(".");
                numberString = numberString.substr(0, decimalPos + 3);
                float frameTextOffset = static_cast<float>(std::floor(ImGui::CalcTextSize(numberString.c_str()).x / 2));

                drawList->AddText(ImVec2(lineStart.x - frameTextOffset, lineStart.y), IM_COL32_WHITE, numberString.c_str());

                lineEnd.y += timelineRegionMax.y - headerSize.y;
                lineStart.y += headerSize.y * 0.5f;
                drawList->AddLine(lineStart, lineEnd, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_Border));
            }
            else
            {
                lineStart.y += headerSize.y * 0.65f;
                drawList->AddLine(lineStart, lineEnd, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_Border));
            }
        }

        // draw currentFrame line if within range
        if (currentKeyFrame >= visibleStartingFrame && currentKeyFrame <= visibleEndingFrame)
        {
            auto frameLineStart = timelineRegionMin;
            frameLineStart.x += GetTimelinePosFromFrame(currentKeyFrame);
            frameLineStart.y += headerSize.y * 0.5f;

            auto frameLineEnd = frameLineStart;
            frameLineEnd.y += timelineRegionMax.y;

            drawList->AddLine(frameLineStart, frameLineEnd, IM_COL32(255, 192, 203, 255));

            auto radius = 5;
            frameLineStart.y += radius;
            drawList->AddCircleFilled(frameLineStart, radius, IM_COL32(255, 192, 203, 255));
        }
    }
    ImGui::PopClipRect();

    //draw separator
    float separatorY = timelineRegionMin.y + headerSize.y;
    drawList->AddLine(ImVec2(ImGui::GetWindowPos().x, separatorY), ImVec2(timelineRegionMin.x + contentRegion.x, separatorY), ImGui::GetColorU32(ImGuiCol_::ImGuiCol_Border));

    if (isPanningTimeline)
    {
        ImVec2 start = ImVec2(timelineRegionMin.x, timelineRegionMin.y - style.WindowPadding.y);
        ImVec2 size = ImVec2(lineStartOffset + 8, timelineRegionMin.y + contentRegion.y + style.ItemSpacing.y * 2);

        drawList->AddRectFilledMultiColor(start, ImVec2(start.x + size.x, start.y + size.y), 0xFF000000, 0u, 0u, 0xFF000000);
    }
}

int AnimationTimelineView::GetFrameFromTimelinePos(float pos)
{
    return static_cast<int>(std::floor((pos - lineStartOffset) / pixelsPerFrame + 0.5f)) + visibleStartingFrame;
}

float AnimationTimelineView::GetTimelinePosFromFrame(int frame)
{
    return (frame - visibleStartingFrame) * pixelsPerFrame + lineStartOffset;
}
