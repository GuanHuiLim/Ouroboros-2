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
bool AnimationTimelineView::playAnim = false;

void AnimationTimelineView::Show()
{
    auto& selected_items = Hierarchy::GetSelected();    //temporary fix, ideally wanna get from animatorcontroller file
    auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();

    if (selected_items.size() <= 0)
        return;

    auto gameObject = scene->FindWithInstanceID(*selected_items.begin());

    if (gameObject == nullptr || gameObject->HasComponent<oo::AnimationComponent>() == false)
        return;

    go = gameObject;
    animator = &go.get()->GetComponent<oo::AnimationComponent>();

    DisplayAnimationTimeline(animator);
}

void AnimationTimelineView::DisplayAnimationTimeline(oo::AnimationComponent* _animator)
{
    DrawTimeLineInfo();

    ImGui::NewLine();

    ImGui::BeginChildFrame(1, ImVec2(0, 0));
    {
        ImGuiStyle& style = ImGui::GetStyle();

        DrawToolbar();
        
        DrawNodeSelector(_animator);

        DrawTimelineSelector(animation);

        ImGui::SameLine();

        DrawTimeLine(animation, style.ItemSpacing.y, ImGui::GetItemRectSize().y);

        DrawTimeLineContent();
    }
    ImGui::EndChildFrame();

    if (playAnim)
    {
        currentKeyFrame++;
    }
}

void AnimationTimelineView::DrawTimeLineInfo()
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
}

void AnimationTimelineView::DrawToolbar()
{
    if (ImGui::Button("<<"))
    {
        currentKeyFrame = visibleStartingFrame;
    }

    ImGui::SameLine();

    if (ImGui::Button("|<"))
    {
        currentKeyFrame -= 1;
        if (currentKeyFrame <= 0)
            currentKeyFrame = 0;
    }

    ImGui::SameLine();

    if (ImGui::Button("Play"))
    {
        playAnim = !playAnim;
    }

    ImGui::SameLine();

    if (ImGui::Button(">|"))
    {
        currentKeyFrame += 1;
    }

    ImGui::SameLine();

    if (ImGui::Button(">>"))
    {
        currentKeyFrame = visibleEndingFrame;
    }

    ImGui::SameLine();

    if (ImGui::Button("Add Keyframe"))
    {
        //provide a drop down of the type of keyframe to make

        if (timeline != nullptr)
        {
            oo::Anim::KeyFrame newkf{
            .data{glm::vec3{0.f,0.f,0.f}},
            .time{currentTime}
            };

            auto& temp = animator->GetActualComponent().animTree->groups;
            auto newKeyFrame = animator->AddKeyFrame(temp.begin()->second.name, node->name, timeline->name, newkf);
            assert(newKeyFrame);
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Add Event"))
    {
        //Creates an event, basically drawing a new keyframe at that currentTime
        if (animation != nullptr)
        {
            oo::Anim::ScriptEvent newEvent{
                .script_function_info{},
                .time{currentTime}
            };

            animation->events.push_back(newEvent);
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Delete KeyFrame"))
    {
        if (timeline != nullptr)
        {
            for (int i = 0; i < timeline->keyframes.size(); ++i)
            {
                if (currentTime == timeline->keyframes[i].time)
                {
                    timeline->keyframes.erase(timeline->keyframes.begin() + i);
                }
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Delete Event"))
    {
        //delete event
        if (animation != nullptr)
        {
            for (int i = 0; i < animation->events.size(); ++i)
            {
                if (currentTime == animation->events[i].time)
                    animation->events.erase(animation->events.begin() + i);
            }
        }
    }
}

void AnimationTimelineView::DrawNodeSelector(oo::AnimationComponent* _animator)
{
    static bool animopen = false;
    static std::string animName = "empty";
    auto& temp = _animator->GetActualComponent().animTree->groups;

    //To Get Which Node to Modify, Temporary Solution
    ImGui::PushItemWidth(160);
    ImGui::BeginGroup();
    ImGui::Text("Node");
    ImGui::SameLine(64.0f);
    ImGui::InputText("##animationstuff", &animName, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::ArrowButton("animdownbtn", ImGuiDir_Down))
    {
        animopen = !animopen;
    }
    if (animopen)
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
                        node = &it2->second;
                        animation = &node->GetAnimation();
                        currentKeyFrame = 0;
                        animopen = !animopen;
                    }
                }
            }
        }
        ImGui::EndListBox();
    }
    ImGui::EndGroup();
    ImGui::PopItemWidth();
}

void AnimationTimelineView::DrawTimelineSelector(oo::Anim::Animation* _animation)
{
    static bool timelineopen = false;
    static std::string timelineName = "empty";
    ImGui::PushItemWidth(160);
    ImGui::BeginGroup();
    ImGui::Text("Timeline");
    ImGui::SameLine();
    ImGui::InputText("##timelinestuff", &timelineName, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::ArrowButton("timelinedownbtn", ImGuiDir_Down))
    {
        timelineopen = !timelineopen;
    }
    if (timelineopen)
    {
        if (ImGui::BeginListBox("##timeline"))
        {
            if (_animation != nullptr)
            {
                for (int i = 0; i < _animation->timelines.size(); ++i)
                {
                    if (ImGui::Selectable(_animation->timelines[i].name.c_str()))
                    {
                        timelineName = _animation->timelines[i].name;
                        timeline = &_animation->timelines[i];
                        timelineopen = !timelineopen;
                    }
                }
            }
            else
            {
                if (ImGui::Selectable("empty"))
                {
                    timelineopen = !timelineopen;
                }
            }
        }
        ImGui::EndListBox();
    }
    ImGui::EndGroup();
    ImGui::PopItemWidth();
}

void AnimationTimelineView::DrawTimeLine(oo::Anim::Animation* _animation, float headerYPadding, float headerHeight)
{
    auto drawList = ImGui::GetWindowDrawList();
    const auto& style = ImGui::GetStyle();
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
            auto hoveringFrame = GetFrameFromTimelinePos(ImGui::GetMousePos().x - timelineRegionMin.x);
            ImGui::BeginTooltip();
            size_t decimalPointPos = std::to_string(hoveringFrame * unitPerFrame).find_first_of(".");
            std::string hoverFrameText = std::to_string(hoveringFrame * unitPerFrame).substr(0, decimalPointPos + 3);
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

void AnimationTimelineView::DrawTimeLineContent()
{
    ImGui::BeginChild("##content", ImVec2(0, 0), false);
    {
        //Add Property Button
        //Draw Keyframes
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
        ImGui::Columns(2, "##legend", false);
        ImGui::SetColumnWidth(0, ImGui::CalcTextSize("Timeline ").x + 160);

        ImGui::NextColumn();

        //Draw Key Frames
        if (timeline != nullptr)
        {
            for (int i = 0; i < timeline->keyframes.size(); ++i)
            {
                for (int j = visibleStartingFrame; j < visibleEndingFrame; ++j)
                {
                    if ((j * unitPerFrame) == timeline->keyframes[i].time)
                    {
                        DrawKeyFrame(j, IM_COL32(211, 211, 211, 255));
                    }
                }
            }
        }

        if (animation != nullptr)
        {
            for (int i = 0; i < animation->events.size(); ++i)
            {
                for (int j = visibleStartingFrame; j < visibleEndingFrame; ++j)
                {
                    if ((j * unitPerFrame) == animation->events[i].time)
                    {
                        DrawKeyFrame(j, IM_COL32(211, 211, 0, 255));
                    }
                }
            }
        }

        ImGui::NextColumn();

        //Properties GUI
        if (timeline != nullptr)
        {
            bool open = ImGui::TreeNodeEx("Position", ImGuiTreeNodeFlags_DefaultOpen);
            
            if (open)
            {
                for (int i = 0; i < timeline->keyframes.size(); ++i)
                {
                    if (currentTime == timeline->keyframes[i].time)
                    {
                        if (timeline->keyframes[i].data.get_type() == rttr::type::get<glm::vec3>())
                        {
                            ImGui::DragFloat("X", &timeline->keyframes[i].data.get_value<glm::vec3>().x);
                            ImGui::DragFloat("Y", &timeline->keyframes[i].data.get_value<glm::vec3>().y);
                            ImGui::DragFloat("Z", &timeline->keyframes[i].data.get_value<glm::vec3>().z);
                        }
                        //else if (timeline->keyframes[i].data.get_type() == rttr::type::get<glm::quat>())
                        //{
                        //    ImGui::DragFloat("W", &timeline->keyframes[i].data.get_value<glm::quat>().w);
                        //    ImGui::DragFloat("X", &timeline->keyframes[i].data.get_value<glm::quat>().x);
                        //    ImGui::DragFloat("Y", &timeline->keyframes[i].data.get_value<glm::quat>().y);
                        //    ImGui::DragFloat("Z", &timeline->keyframes[i].data.get_value<glm::quat>().z);
                        //}
                        //else if (timeline->keyframes[i].data.get_type() == rttr::type::get<bool>())
                        //{
                        //    ImGui::Checkbox("Bool", &timeline->keyframes[i].data.get_value<bool>());
                        //}
                    }
                }

                ImGui::NextColumn();
                for (int i = 0; i < timeline->keyframes.size(); ++i)
                {
                    for (int j = visibleStartingFrame; j < visibleEndingFrame; ++j)
                    {
                        if ((j * unitPerFrame) == timeline->keyframes[i].time)
                        {
                            if (timeline->keyframes[i].data.get_type() == rttr::type::get<glm::vec3>())
                            {
                                ImGui::SetCursorPosY(25);
                                DrawKeyFrame(j, IM_COL32(0, 211, 0, 255));
                                ImGui::SetCursorPosY(48);
                                DrawKeyFrame(j, IM_COL32(0, 211, 0, 255));
                                ImGui::SetCursorPosY(71);
                                DrawKeyFrame(j, IM_COL32(0, 211, 0, 255));
                            }
                            //else if (timeline->keyframes[i].data.get_type() == rttr::type::get<glm::quat>())
                            //{
                            //    ImGui::SetCursorPosY(25);
                            //    DrawKeyFrame(j, IM_COL32(0, 211, 0, 255));
                            //    ImGui::SetCursorPosY(48);
                            //    DrawKeyFrame(j, IM_COL32(0, 211, 0, 255));
                            //    ImGui::SetCursorPosY(71);
                            //    DrawKeyFrame(j, IM_COL32(0, 211, 0, 255));
                            //    ImGui::SetCursorPosY(94);
                            //    DrawKeyFrame(j, IM_COL32(0, 211, 0, 255));
                            //}
                            //else if (timeline->keyframes[i].data.get_type() == rttr::type::get<bool>())
                            //{
                            //    ImGui::SetCursorPosY(25);
                            //    DrawKeyFrame(j, IM_COL32(0, 211, 0, 255));
                            //}
                        }
                    }
                }
                ImGui::TreePop();
            }
            ImGui::NewLine();
            ImGui::Separator();
        }

        ImGui::EndChild();
    }
    DisplayEventInspector(animation);

}

void AnimationTimelineView::DisplayEventInspector(oo::Anim::Animation* _animation)
{
    if (animation != nullptr)
    {
        if (ImGui::Begin("Animation Event Inspector", &displayEventInspector))
        {
            for (int i = 0; i < animation->events.size(); ++i)
            {
                if (currentTime == animation->events[i].time)
                {
                    ImGui::Text("Invoke: ");
                }
            }
            ImGui::End();
        }
    }
}

void AnimationTimelineView::DrawKeyFrame(int _currentKeyFrame, ImU32 colour)
{
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    float keyFrameSize = 7.0f;
    int halfKeyFrameSize = static_cast<int>(std::floor(keyFrameSize / 2));

    cursorPos.x += GetTimelinePosFromFrame(_currentKeyFrame) + (halfKeyFrameSize + 22);
    cursorPos.y -= 2;

    ImVec2 size = ImVec2(keyFrameSize, keyFrameSize + 4);

    ImGui::GetWindowDrawList()->AddRectFilled(cursorPos, ImVec2(cursorPos.x + size.x, cursorPos.y + size.y), colour);
}

int AnimationTimelineView::GetFrameFromTimelinePos(float pos)
{
    return static_cast<int>(std::floor((pos - lineStartOffset) / pixelsPerFrame + 0.5f)) + visibleStartingFrame;
}

float AnimationTimelineView::GetTimelinePosFromFrame(int frame)
{
    return (frame - visibleStartingFrame) * pixelsPerFrame + lineStartOffset;
}


