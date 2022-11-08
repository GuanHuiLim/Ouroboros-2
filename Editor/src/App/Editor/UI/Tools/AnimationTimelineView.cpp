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
bool AnimationTimelineView::opentimeline = false;

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

        ImGui::SameLine();

        DrawTimeLine(animation, style.ItemSpacing.y, ImGui::GetItemRectSize().y);

        DrawTimeLineContent();
    }
    ImGui::EndChildFrame();

    if (playAnim)
    {
        if (animation != nullptr)
        {
            //TODO: Loop Animation Keyframe
            //stores the keyframs and timelines in that particular keyframe
            for (int i = 0; i < animation->timelines.size(); ++i)
            {
                for (int j = 0; j < animation->timelines[i].keyframes.size(); ++j)
                {
                    if ((currentKeyFrame * unitPerFrame) == animation->timelines[i].keyframes[j].time)
                    {
                        if (!keyframes.empty())
                            keyframes.clear();
                        if (!timelines.empty())
                            timelines.clear();

                        timelines.push_back(&animation->timelines[i]);
                        keyframes.push_back(&animation->timelines[i].keyframes[j]);
                    }
                }
            }

            //applies the data of the stored timelines and keyframes to their respective rttr::instance object
            for (int i = 0; i < timelines.size(); ++i)
            {
                if (timelines[i]->datatype == oo::Anim::Timeline::DATATYPE::VEC3)
                {
                    //apply it according to the rttr_property
                    timelines[i]->rttr_property.set_value(go.get()->GetComponent<oo::TransformComponent>(),
                                                          keyframes[i]->data.get_value<glm::vec3>());
                }
            }

            currentKeyFrame++;
        }
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
        currentKeyFrame = static_cast<int>(visibleStartingFrame);
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
        currentKeyFrame = static_cast<int>(visibleEndingFrame);
    }

    ImGui::SameLine();

    if (ImGui::Button("Add Keyframe"))
    {
        //provide a drop down of the type of keyframe to make

        if (timeline != nullptr)
        {
            oo::Anim::KeyFrame newkf = oo::Anim::KeyFrame(glm::vec3{ 0.f,0.f,0.f }, currentTime);

            auto& temp = animator->GetActualComponent().animTree->groups;
            for (auto it = temp.begin(); it != temp.end(); ++it)
            {
                auto newKeyFrame = animator->AddKeyFrame(it->second.name, node->name, timeline->name, newkf);
                assert(newKeyFrame);
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Add Event"))
    {
        //Creates an event, basically drawing a new keyframe at that currentTime
        if (animation != nullptr)
        {
            oo::Anim::ScriptEvent newEvent = oo::Anim::ScriptEvent();

            newEvent.script_function_info = oo::ScriptValue::function_info() ;
            newEvent.time = currentTime;

            animation->events.push_back(newEvent);
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Delete KeyFrame"))
    {
        if (animation != nullptr)
        {
            for (int i = 0; i < animation->timelines.size(); ++i)
            {
                for (int j = 0; j < animation->timelines[i].keyframes.size(); ++j)
                {
                    if (currentTime == animation->timelines[i].keyframes[j].time)
                    {
                        animation->timelines[i].keyframes.erase(animation->timelines[i].keyframes.begin() + j);
                    }
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
            if (_animator->GetActualComponent().animTree != nullptr)
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

    visibleEndingFrame = static_cast<float>(GetFrameFromTimelinePos(headerSize.x));

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
        int frames = static_cast<int>(visibleEndingFrame - visibleStartingFrame);
        for (int f = 0; f < frames; ++f)
        {
            int frame = f + static_cast<int>(visibleStartingFrame);
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

            float radius = 5.0f;
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
        ImVec2 size = ImVec2(static_cast<float>(lineStartOffset + 8), timelineRegionMin.y + contentRegion.y + style.ItemSpacing.y * 2);

        drawList->AddRectFilledMultiColor(start, ImVec2(start.x + size.x, start.y + size.y), 0xFF000000, 0u, 0u, 0xFF000000);
    }
}

void AnimationTimelineView::DrawTimeLineContent()
{
    ImGui::BeginChild("##content", ImVec2(0, 0), false);
    {
        //Add Property Button
        //Draw Keyframes
        static int currentTimeLineCount = 0;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
        ImGui::Columns(2, "##legend", false);
        ImGui::SetColumnWidth(0, ImGui::CalcTextSize("Timeline ").x + 160);

        ImGui::NextColumn();

        if (animation != nullptr)
        {
            //Draw Timelines
            for (int i = 0; i < animation->timelines.size(); ++i)
            {
                for (int j = 0; j < animation->timelines[i].keyframes.size(); ++j)
                {
                    for (int k = static_cast<int>(visibleStartingFrame); k < static_cast<int>(visibleEndingFrame); ++k)
                    {
                        if ((k * unitPerFrame) == animation->timelines[i].keyframes[j].time)
                        {
                            DrawKeyFrame(k, { 0, 0, 0, 255 }, i * 20.0f, "K");
                            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
                            {
                                keyframe = &animation->timelines[i].keyframes[j];
                                timeline = &animation->timelines[i];
                                currentKeyFrame = k;
                            }
                        }
                    }
                }
            }

            //Draw Event
            for (int i = 0; i < animation->events.size(); ++i)
            {
                for (int j = static_cast<int>(visibleStartingFrame); j < static_cast<int>(visibleEndingFrame); ++j)
                {
                    if ((j * unitPerFrame) == animation->events[i].time)
                    {
                        DrawKeyFrame(j, { 211, 211, 0, 255 }, -2.0f, "E");
                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
                        {
                            scriptevent = &animation->events[i];
                            currentKeyFrame = j;
                        }
                    }
                }
            }
        }

        ImGui::NextColumn();

        //Properties GUI
        if (animation != nullptr)
        {
            //do for multiple timeline
            for (int i = 0; i < animation->timelines.size(); ++i)
            {
                ImGui::PushID(i);
                ImGui::SetCursorPosY(5.0f + (i * 20.0f));
                bool requestDelete = false;
                if (ImGui::SmallButton("X"))
                {
                    //used for deleting animation timelines
                    requestDelete = true;
                }

                ImGui::SameLine();

                if (!animation->timelines.empty())
                {
                    if (ImGui::Selectable(animation->timelines[i].name.c_str(), false))
                    {
                        timeline = &animation->timelines[i];
                    }
                    //opentimeline = ImGui::Selectable(animation->timelines[i].name.c_str(), false);

                    //if (opentimeline)
                    //{
                    //    timeline = &animation->timelines[i];
                    //    for (int j = 0; j < animation->timelines[i].keyframes.size(); ++j)
                    //    {
                    //        if (animation->timelines[i].datatype == oo::Anim::Timeline::DATATYPE::VEC3)
                    //        {
                    //            if (currentTime == animation->timelines[i].keyframes[j].time)
                    //            {
                    //                //draw the property editor
                    //                //ImGui::DragFloat("X", &animation->timelines[i].keyframes[j].data.get_value<glm::vec3>().x);
                    //                //ImGui::DragFloat("Y", &animation->timelines[i].keyframes[j].data.get_value<glm::vec3>().y);
                    //                //ImGui::DragFloat("Z", &animation->timelines[i].keyframes[j].data.get_value<glm::vec3>().z);

                    //                //animation->timelines[i].rttr_property.set_value(go.get()->GetComponent<oo::TransformComponent>(),
                    //                //                                                animation->timelines[i].keyframes[j].data.get_value<glm::vec3>());
                    //            }
                    //        }
                    //    }
                    //    ImGui::TreePop();
                    //}
                }
                if(requestDelete)
                    animation->timelines.erase(animation->timelines.begin() + i);

                ImGui::PopID();
            }
            ImGui::NewLine();
            ImGui::Separator();

            if (ImGui::Button("Add Timeline"))
            {
                //create popup that shows the transform and gameObject child hierarchy
                ImGui::OpenPopup("##addtimeline");
            }

            if (ImGui::BeginPopup("##addtimeline"))
            {
                if (ImGui::MenuItem("Add Example Timeline"))
                {
                    auto& temp = animator->GetActualComponent().animTree->groups;

                    for (auto it = temp.begin(); it != temp.end(); ++it)
                    {
                        oo::Anim::TimelineInfo exampleTimeline{
                        .type{oo::Anim::Timeline::TYPE::PROPERTY},
                        .component_hash{Ecs::ECSWorld::get_component_hash<oo::TransformComponent>()},
                        .rttr_property{rttr::type::get< oo::TransformComponent>().get_property("Position")},
                        .timeline_name{"Example Timeline " + std::to_string(currentTimeLineCount)},
                        .target_object{*(go.get())},
                        .source_object{*(go.get())}
                        };
                        auto exampleTL = animator->AddTimeline(it->second.name, node->name, exampleTimeline);
                        timeline = exampleTL.operator->();
                    }
                    ++currentTimeLineCount;
                }
                ImGui::EndPopup();
            }
        }

        ImGui::EndChild();
    }
    DisplayInspector();
}

void AnimationTimelineView::DisplayInspector()
{
        if (ImGui::Begin("Animation Event Inspector"))
        {
            if (scriptevent != nullptr)
            {
                ImGui::Text("Invoke: ");
                ImGui::Text("Time: ");
                ImGui::SameLine();
                ImGui::DragFloat("##scripteventtime", &scriptevent->time, unitPerFrame);
            }
            ImGui::End();
        }


        if (ImGui::Begin("Animation Timeline Inspector"))
        {
            if (timeline != nullptr)
            {
                ImGui::Text("Name: ");
                ImGui::SameLine();
                ImGui::InputText("##timelineName", &timeline->name, ImGuiInputTextFlags_ReadOnly);
            }
            ImGui::End();
        }

        if (ImGui::Begin("KeyFrame Editor"))
        {
            if (keyframe != nullptr)
            {
                ImGui::Text("Time: ");
                ImGui::SameLine();
                ImGui::DragFloat("##keyframetime", &keyframe->time, unitPerFrame);
                if (currentTime == keyframe->time)
                {
                    if (timeline != nullptr && timeline->datatype == oo::Anim::Timeline::DATATYPE::VEC3)
                    {
                        //draw the property editor
                        ImGui::DragFloat("X", &keyframe->data.get_value<glm::vec3>().x);
                        ImGui::DragFloat("Y", &keyframe->data.get_value<glm::vec3>().y);
                        ImGui::DragFloat("Z", &keyframe->data.get_value<glm::vec3>().z);

                        //need to support applying to child object
                        timeline->rttr_property.set_value(go.get()->GetComponent<oo::TransformComponent>(),
                                                            keyframe->data.get_value<glm::vec3>());
                    }
                }
            }
            ImGui::End();
        }
}

void AnimationTimelineView::DrawKeyFrame(int _currentKeyFrame, const ImVec4& colour, float ypos, const std::string& label)
{
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    float keyFrameSize = 7.0f;
    int halfKeyFrameSize = static_cast<int>(std::floor(keyFrameSize / 2));

    cursorPos.x += GetTimelinePosFromFrame(_currentKeyFrame) + (halfKeyFrameSize + 22);
    cursorPos.y -= 2;

    //ImVec2 size = ImVec2(keyFrameSize, keyFrameSize + 4);

    float screenx = cursorPos.x;
    float x = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(screenx - (1.635f * x));
    ImGui::SetCursorPosY(ypos);
    ImGui::PushStyleColor(ImGuiCol_Button, colour);
    ImGui::Button(label.c_str());
    ImGui::PopStyleColor();
}

int AnimationTimelineView::GetFrameFromTimelinePos(float pos)
{
    return static_cast<int>(std::floor((pos - lineStartOffset) / pixelsPerFrame + 0.5f)) + static_cast<int>(visibleStartingFrame);
}

float AnimationTimelineView::GetTimelinePosFromFrame(int frame)
{
    return (frame - visibleStartingFrame) * pixelsPerFrame + lineStartOffset;
}