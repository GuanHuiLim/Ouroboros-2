/************************************************************************************//*!
\file          AnimatorControllerView.cpp
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 22, 2022
\brief         File Contains the definition needed to create an Animator Controller View
               for the engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "Project.h"
#include "../Object Editor/AssetBrowser.h"
#include "AnimatorControllerView.h"
#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/Scene/Scene.h>
#include "App/Editor/UI/Object Editor/Hierarchy.h"
#include "Ouroboros/Animation/AnimationSystem.h"

#include "App/Editor/Utility/ImGuiManager.h"

uintptr_t GetNextId()
{
    static uintptr_t id = 1;
    return id++;
}

AnimatorControllerView::NodeInfo* AnimatorControllerView::FindNode(ed::PinId pinID)
{
    for (auto& node : m_nodes)
    {
        if (node.Input[0].id == pinID)
            return &node;
        else if (node.Output[0].id == pinID)
            return &node;
    }
    return nullptr;
}

AnimatorControllerView::NodeInfo* AnimatorControllerView::FindNode(ed::NodeId id)
{
    for (auto& node : m_nodes)
        if (node.id == id)
            return &node;

    return nullptr;
}

AnimatorControllerView::NodeInfo* AnimatorControllerView::FindNode(oo::Anim::Node* _node)
{
    auto id = reinterpret_cast<uintptr_t>(static_cast<void*>(_node));
    for (auto& node : m_nodes)
        if (node.id.Get() == id)
            return &node;
    return nullptr;
}

void AnimatorControllerView::ReturnID(ed::NodeId id)
{
    free_Node_IDs.emplace(id.Get());
}
void AnimatorControllerView::ReturnID(ed::PinId id)
{
    free_Node_IDs.emplace(id.Get());
}
void AnimatorControllerView::ReturnID(ed::LinkId id)
{
    free_Node_IDs.emplace(id.Get());
}

uintptr_t AnimatorControllerView::GetAvailableNodeID()
{
    if (free_Node_IDs.empty()) return GetNextId();

    auto id = free_Node_IDs.top();
    free_Node_IDs.pop();
    return id;
}

void AnimatorControllerView::Show()
{
    //Get Info From the GameObject
    auto& selected_items = Hierarchy::GetSelected();    //temporary fix, ideally wanna get from animatorcontroller file
    auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();

    if (selected_items.size() <= 0)
    {
        Clear();
        return;
    }

    auto gameobject = scene->FindWithInstanceID(*selected_items.begin());

    if (gameobject == nullptr || gameobject->HasComponent<oo::AnimationComponent>() == false)
    {
        Clear();
        return;
    }

    animator = &gameobject.get()->GetComponent<oo::AnimationComponent>();

    LoadGraph(animator);

    DisplayAnimatorController(animator);

    PostDisplayUpdate(animator);
}

void AnimatorControllerView::BuildNode(NodeInfo* node)
{
    assert(node);
    if (node == nullptr) return;

    for (auto& input : node->Input)
    {
        input.node = node;
        input.kind = PinKind::Input;
    }

    for (auto& output : node->Output)
    {
        output.node = node;
        output.kind = PinKind::Output;
    }

    BuildNodeOnEditor(*node);
}

void AnimatorControllerView::DisplayAnimatorController(oo::AnimationComponent* _animator)
{
    ed::SetCurrentEditor(m_Context);
    ed::Begin("Animator Controller Editor", ImVec2(0.0, 0.0));

    if (!_animator || _animator->HasAnimationTree() == false)
    {
        ed::End();
        return;
    }

    //if (!_animator->GetActualComponent().animTree)
    //{
    //    auto tree = oo::Anim::AnimationSystem::CreateAnimationTree("Test Animation Tree");
    //    (void)(tree);
    //    _animator->SetAnimationTree("Test Animation Tree");
    //    //auto& start_node = tree->groups.begin()->second.startNode;
    //}


    //
    // 1) Commit known data to editor
    //

    //for (int i = 0; i < m_nodes.size(); ++i)
    //{
    //    //if (m_firstFrame)
    //    ed::SetNodePosition(m_nodes[i].id, m_nodes[i].pos);
    //    ed::BeginNode(m_nodes[i].id);
    //    ed::BeginPin(m_nodes[i].Input[0].id, ed::PinKind::Input);
    //    ImGui::Text("O");
    //    ed::EndPin();
    //    ImGui::SameLine();
    //    ImGui::Text(m_nodes[i].anim_node->name.c_str());
    //    ImGui::SameLine();
    //    ed::BeginPin(m_nodes[i].Output[0].id, ed::PinKind::Output);
    //    ImGui::Text(">");
    //    ed::EndPin();
    //    ed::EndNode();
    //}
    for (auto& nodeinfo : m_nodes)
    {
        BuildNodeOnEditor(nodeinfo);
    }


    for (auto& LinkInfo : m_links_)
    {
        ed::Link(LinkInfo.id, LinkInfo.inputID, LinkInfo.outputID);
    }

    //
    // 2) Handle interactions
    //

    // Handle creation action, returns true if editor want to create new object (node or link)
    if (ed::BeginCreate())
    {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId))
        {
            // QueryNewLink returns true if editor want to create new link between pins.
            //
            // Link can be created only for two valid pins, it is up to you to
            // validate if connection make sense. Editor is happy to make any.
            //
            // Link always goes from input to output. User may choose to drag
            // link from output pin or input pin. This determine which pin ids
            // are valid and which are not:
            //   * input valid, output invalid - user started to drag new ling from input pin
            //   * input invalid, output valid - user started to drag new ling from output pin
            //   * input valid, output valid   - user dragged link over other pin, can be validated

            if (inputPinId && outputPinId) // both are valid, let's accept link
            {
                // ed::AcceptNewItem() return true when user release mouse button.
                if (ed::AcceptNewItem())
                {
                    auto link = _animator->AddLink(current_group_name,
                        FindNode(inputPinId)->anim_node->name,
                        FindNode(outputPinId)->anim_node->name);

                    // Since we accepted new link, lets add one to our list of links.
                    auto linkinfo = CreateLink(link.operator->(), inputPinId, outputPinId);

                    // Draw new link.
                    ed::Link(linkinfo->id, linkinfo->inputID, linkinfo->outputID);
                }

                // You may choose to reject connection between these nodes
                // by calling ed::RejectNewItem(). This will allow editor to give
                // visual feedback by changing link thickness and color.
            }
        }
    }
    ed::EndCreate(); // Wraps up object creation action handling.

    // Handle deletion action
    OnDelete();

    ed::Suspend();
    if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("Animator Editor Popup");
    }
    ed::Resume();

    ed::Suspend();
    if (ImGui::BeginPopup("Animator Editor Popup"))
    {
        if (ImGui::MenuItem("Create State"))
        {
            //Need to find a way to store the oo::Anim::NodeInfo
            //Temporary Solution
            auto& temp = _animator->GetActualComponent().animTree->groups;
            for (auto it = temp.begin(); it != temp.end(); ++it)
            {
                if (it == temp.begin())
                {
                    oo::Anim::NodeInfo nodeinfo{
                        .name{ "New Node" /*+ std::to_string(uniqueId)*/ },
                        .animation_name{ oo::Anim::internal::empty_animation_name },
                        .speed{ 1.f },
                        .position{0.f,0.f,0.f}
                    };

                    auto node = _animator->AddNode(it->second.name, nodeinfo);
                    assert(node);
                    if (node) CreateNode(node.operator->());
                }
            }
        }
        ImGui::EndPopup();
    }
    ed::Resume();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
        for (auto& link : m_links_)
            ed::Flow(link.id);

    // End of interaction with imgui node editor.
    ed::End();

    //MUST DO THIS TO MOVE THE NODES
    m_firstFrame = false;

    DisplayInspector();
    DisplayParameters();
}

void AnimatorControllerView::DisplayParameters()
{
    static std::string _paramTypes[] = { "BOOL","TRIGGER","INT","FLOAT" };
    static bool open4 = false;
    static int _paramid = 0;
    int _currParamId = 0;

    if (ImGui::Begin("Parameters"))
    {
        ImVec2 textsize = ImGui::CalcTextSize("a");

        if (!animator->GetActualComponent().animTree->parameters.empty())
        {
            for (int i = 0; i < animator->GetActualComponent().animTree->parameters.size(); ++i)
            {
                if (animator->GetActualComponent().animTree->parameters.size() != 0)
                {
                    bool requestDelete = false;
                    ++_currParamId;
                    ImGui::PushID(static_cast<int>(animator->GetActualComponent().animTree->parameters[i].paramID));
                    ImGui::PushItemWidth(-160);
                    if (ImGui::SmallButton("X"))
                    {
                        //remove parameter from the vector
                        requestDelete = true;
                    }
                    ImGui::PopItemWidth();

                    ImGui::SameLine();

                    ImGui::PushItemWidth(-160);
                    if (!animator->GetActualComponent().animTree->parameters.empty())
                    {
                        auto temp_name = animator->GetActualComponent().animTree->parameters[i].name;
                        if (ImGui::InputText("##Name", &temp_name, ImGuiInputTextFlags_EnterReturnsTrue) == true)
                        {
                            animator->GetActualComponent().animTree->parameters[i].name = temp_name;
                        }
                    }
                    ImGui::PopItemWidth();

                    ImGui::SameLine();

                    ImGui::PushItemWidth(-80);
                    ImGui::BeginGroup();
                    if (!animator->GetActualComponent().animTree->parameters.empty())
                    {
                        ImGui::InputText("##paramstuff", &_paramTypes[(int)animator->GetActualComponent().animTree->parameters[i].type], ImGuiInputTextFlags_ReadOnly);
                        ImGui::SameLine();
                        if (ImGui::ArrowButton("downbtn", ImGuiDir_Down))
                        {
                            _paramid = _currParamId;
                            open4 = !open4;
                        }
                        if (open4 && _currParamId == _paramid)
                        {
                            if (ImGui::BeginListBox("##params"))
                            {
                                if (ImGui::Selectable(_paramTypes[0].c_str()))
                                {
                                    animator->GetActualComponent().animTree->parameters[i].type = oo::Anim::P_TYPE::BOOL;
                                    animator->GetActualComponent().animTree->parameters[i].value = false;
                                    open4 = !open4;
                                }
                                if (ImGui::Selectable(_paramTypes[1].c_str()))
                                {
                                    animator->GetActualComponent().animTree->parameters[i].type = oo::Anim::P_TYPE::TRIGGER;
                                    animator->GetActualComponent().animTree->parameters[i].value = false;
                                    open4 = !open4;
                                }
                                if (ImGui::Selectable(_paramTypes[2].c_str()))
                                {
                                    animator->GetActualComponent().animTree->parameters[i].type = oo::Anim::P_TYPE::INT;
                                    animator->GetActualComponent().animTree->parameters[i].value = 0;
                                    open4 = !open4;
                                }
                                if (ImGui::Selectable(_paramTypes[3].c_str()))
                                {
                                    animator->GetActualComponent().animTree->parameters[i].type = oo::Anim::P_TYPE::FLOAT;
                                    animator->GetActualComponent().animTree->parameters[i].value = 0.0f;
                                    open4 = !open4;
                                }
                            }
                            ImGui::EndListBox();
                        }
                    }
                    ImGui::EndGroup();
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::PushItemWidth(-1);
                    if (!animator->GetActualComponent().animTree->parameters.empty())
                    {
                        switch (animator->GetActualComponent().animTree->parameters[i].type)
                        {
                            case oo::Anim::P_TYPE::BOOL:
                            {
                                bool temp = animator->GetActualComponent().animTree->parameters[i].value.get_value<bool>();
                                if (ImGui::Checkbox("##bool", &temp))
                                    animator->GetActualComponent().animTree->parameters[i].value = temp;
                                break;
                            }
                            case oo::Anim::P_TYPE::TRIGGER:
                            {
                                bool temp = animator->GetActualComponent().animTree->parameters[i].value.get_value<bool>();
                                if (ImGui::Checkbox("##trigger", &temp))
                                    animator->GetActualComponent().animTree->parameters[i].value = temp;
                                break;
                            }
                            case oo::Anim::P_TYPE::INT:
                            {
                                int temp = animator->GetActualComponent().animTree->parameters[i].value.get_value<int>();
                                if (ImGui::DragInt("##trigger", &temp))
                                    animator->GetActualComponent().animTree->parameters[i].value = temp;
                                break;
                            }
                            case oo::Anim::P_TYPE::FLOAT:
                            {
                                float temp = animator->GetActualComponent().animTree->parameters[i].value.get_value<float>();
                                if (ImGui::DragFloat("##trigger", &temp))
                                    animator->GetActualComponent().animTree->parameters[i].value = temp;
                                break;
                            }
                        }
                    }
                    ImGui::PopItemWidth();
                    ImGui::PopID();
                    ImGui::Separator();

                    if(requestDelete)
                        animator->GetActualComponent().animTree->parameters.erase(animator->GetActualComponent().animTree->parameters.begin() + i);
                }
            }
        }

        if (ImGui::Button("+"))
        {
            ImGui::OpenPopup("##+");
        }

        if (ImGui::BeginPopup("##+"))
        {
            if (ImGui::MenuItem("BOOL"))
            {
                oo::Anim::ParameterInfo param_info{
                .name{"bool"},
                .type{oo::Anim::P_TYPE::BOOL}
                };
                animator->AddParameter(param_info);
            }
            if (ImGui::MenuItem("TRIGGER"))
            {
                oo::Anim::ParameterInfo param_info{
                .name{"trigger"},
                .type{oo::Anim::P_TYPE::TRIGGER}
                };
                animator->AddParameter(param_info);
            }
            if (ImGui::MenuItem("FLOAT"))
            {
                oo::Anim::ParameterInfo param_info{
                .name{"float"},
                .type{oo::Anim::P_TYPE::FLOAT},
                //optional
                .value{ 10.f }
                };
                animator->AddParameter(param_info);
            }
            if (ImGui::MenuItem("INT"))
            {
                oo::Anim::ParameterInfo param_info{
                .name{"Test int"},
                .type{oo::Anim::P_TYPE::INT},
                //optional
                .value{ 10 }
                };
                animator->AddParameter(param_info);
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }
}

void AnimatorControllerView::DisplayInspector()
{
    auto num_selected_objects = ed::GetSelectedObjectCount(); 

    m_nodesId.resize(static_cast<size_t>(num_selected_objects));
    m_linksId.resize(static_cast<size_t>(num_selected_objects));

    int nodeCount = ed::GetSelectedNodes(m_nodesId.data(), num_selected_objects);
    int linkCount = ed::GetSelectedLinks(m_linksId.data(), num_selected_objects);

    if (ImGui::Begin("Animator Inspector"))
    {
        if (nodeCount != 0)
        {
            static ImGuiID open = 0;
            for (int i = 0; i < nodeCount; ++i)
            {
                ed::NodeId temp = m_nodesId[i];
                auto id = std::find_if(m_nodes.begin(), m_nodes.end(), [temp](auto& node) { return node.id == temp; });
                
                //do nothing if no found in m_nodes
                if (id == m_nodes.end()) continue;
                //do nothing if no animation assigned for now
                if (id->anim_node->HasAnimation() == false) continue;

                ImGui::Text("Name");
                ImVec2 textsize = ImGui::CalcTextSize("a");
                ImGui::SameLine(textsize.x * 8);
                ImGui::InputText("##name", &id->anim_node->name);
                ImGui::Separator();
                ImGui::Text("Animation");
                ImGui::SameLine(textsize.x * 12);
                ImGui::InputText("##animation", &id->anim_node->GetAnimation().name, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
                ImGui::SameLine();
                DisplayAnimationSelector(id->anim_node, open);
                ImGui::Text("Speed");
                ImGui::SameLine(textsize.x * 12);
                ImGui::InputFloat("##speed", &id->anim_node->speed);
                ImGui::Text("Looping");
                ImGui::SameLine(textsize.x * 12);
                ImGui::Checkbox("##looping", &id->anim_node->GetAnimation().looping);
            }
        }
        else if (linkCount != 0)
        {
            for (int i = 0; i < linkCount; ++i)
            {
                ed::LinkId temp = m_linksId[i];
                auto id = std::find_if(m_links_.begin(), m_links_.end(), [temp](auto& link) {return link.id == temp; });
                //std::string linkRelation = id->link->name;
                //ImGui::Text(linkRelation.c_str());
                ImVec2 textsize = ImGui::CalcTextSize("a");
                ImGui::Text("Has Exit Time");
                ImGui::SameLine(textsize.x * 25);
                ImGui::Checkbox("##hasexittime", &id->link->has_exit_time);
                if (ImGui::TreeNode("Settings"))
                {
                    {
                        ImGui::Text("Exit Time");
                        ImGui::SameLine(textsize.x * 25);
                        if (id->link->has_exit_time)
                            ImGui::InputFloat("##exitTime", &id->link->exit_time, 0.0f, 0.0f, "%.2f");
                        else
                            ImGui::InputFloat("##exitTime", &id->link->exit_time, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_ReadOnly);
                        ImGui::Text("Fixed Duration");
                        ImGui::SameLine(textsize.x * 25);
                        ImGui::Checkbox("##fixedduration", &id->link->fixed_duration);
                        ImGui::Text("Transition Duration");
                        ImGui::SameLine(textsize.x * 25);
                        ImGui::InputFloat("##transitionduration", &id->link->transition_duration, 0.0f, 0.0f, "%.2f");

                    }
                    ImGui::TreePop();
                }
                ImGui::Separator();
                DisplayConditions(id->link);
            }
        }
        ImGui::End();
    }
}

void AnimatorControllerView::DisplayAnimationSelector(oo::Anim::Node* _anim_node, ImGuiID& openID)
{
    ImGui::PushID(_anim_node->name.c_str());
    ImGuiID tempID = ImGui::GetItemID();
    if (ImGui::Button("Edit"))
    {
        if (openID == tempID)
            openID = 0;
        else
        {
            openID = tempID;
        }
    }
    ImGui::PopID();
    if (openID == tempID)
    {
        //display the animations
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        ImVec2 spacing = ImGui::GetStyle().ItemSpacing;

        if (ImGui::BeginChild("Select Animation", windowSize, true))
        {
            for (const auto& assets : Project::GetAssetManager()->GetAssetsByType(oo::AssetInfo::Type::Animation))
            {
                if (ImGui::Selectable(assets.GetFilePath().stem().string().c_str()))
                {
                    //TODO: CHANGE ANIMATION HERE
                    _anim_node->SetAnimationAsset(assets);
                    //_anim_node->anim_asset = assets;
                    openID = 0;
                }
            }
            ImGui::EndChild();
        }
    }
}

void AnimatorControllerView::DisplayConditions(oo::Anim::Link* link)
{
    static std::string _compareTypes[] = { "GREATER", "LESSER", "EQUAL", "NOT EQUAL" };
    static int _condid = 0;
    int _currCondId = 0;
    static bool open = false;
    ImVec2 textsize = ImGui::CalcTextSize("a");

    ImGui::Text("Conditions");
    ImGui::NewLine();

    if (link == nullptr || animator->GetActualComponent().animTree->parameters.empty())
        return;

    for (int i = 0; i < link->conditions.size(); ++i)
    {
        for (int j = 0; j < animator->GetActualComponent().animTree->parameters.size(); ++j)
        {
            if (link->conditions.size() != 0)
            {
                if (link->conditions[i].paramID == animator->GetActualComponent().animTree->parameters[j].paramID)
                {
                    bool requestDelete = false;
                    //do param display
                    ++_currCondId;
                    ImGui::PushID(static_cast<int>(link->conditions[i].paramID));
                    ImGui::PushItemWidth(-160);
                    if (ImGui::SmallButton("X"))
                    {
                        //remove parameter from the vector
                        requestDelete = true;
                    }
                    ImGui::PopItemWidth();

                    ImGui::SameLine();

                    ImGui::PushItemWidth(-160);
                    auto temp_name = animator->GetActualComponent().animTree->parameters[j].name;
                    if (ImGui::InputText("##Name", &temp_name, ImGuiInputTextFlags_ReadOnly) == true)
                    {
                        animator->GetActualComponent().animTree->parameters[j].name = temp_name;
                    }
                    ImGui::PopItemWidth();

                    ImGui::SameLine();

                    ImGui::PushItemWidth(-80);
                    ImGui::BeginGroup();
                    if (!link->conditions.empty())
                    {
                        ImGui::InputText("##comparestuff", &_compareTypes[(int)link->conditions[i].comparison_type], ImGuiInputTextFlags_ReadOnly);
                        ImGui::SameLine();
                        if (ImGui::ArrowButton("downbtn", ImGuiDir_Down))
                        {
                            _condid = _currCondId;
                            open = !open;
                        }
                        if (open && _currCondId == _condid)
                        {
                            if (ImGui::BeginListBox("##compare"))
                            {
                                if (link->conditions[i].type == oo::Anim::P_TYPE::INT || link->conditions[i].type == oo::Anim::P_TYPE::FLOAT)
                                {
                                    if (ImGui::Selectable(_compareTypes[0].c_str()))
                                    {
                                        link->conditions[i].comparison_type = oo::Anim::Condition::CompareType::GREATER;
                                        open = !open;
                                    }
                                    if (ImGui::Selectable(_compareTypes[1].c_str()))
                                    {
                                        link->conditions[i].comparison_type = oo::Anim::Condition::CompareType::LESS;
                                        open = !open;
                                    }
                                }
                                if (ImGui::Selectable(_compareTypes[2].c_str()))
                                {
                                    link->conditions[i].comparison_type = oo::Anim::Condition::CompareType::EQUAL;
                                    open = !open;
                                }
                                if (ImGui::Selectable(_compareTypes[3].c_str()))
                                {
                                    link->conditions[i].comparison_type = oo::Anim::Condition::CompareType::NOT_EQUAL;
                                    open = !open;
                                }
                            }
                            ImGui::EndListBox();
                        }
                    }
                    ImGui::EndGroup();
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::PushItemWidth(-1);
                    if (!link->conditions.empty())
                    {
                        switch (link->conditions[i].type)
                        {
                        case oo::Anim::P_TYPE::BOOL:
                        {
                            bool temp = link->conditions[i].value.get_value<bool>();
                            if (ImGui::Checkbox("##bool", &temp))
                                link->conditions[i].value = temp;
                            break;
                        }
                        case oo::Anim::P_TYPE::TRIGGER:
                        {
                            bool temp = link->conditions[i].value.get_value<bool>();
                            if (ImGui::Checkbox("##trigger", &temp))
                                link->conditions[i].value = temp;
                            break;
                        }
                        case oo::Anim::P_TYPE::INT:
                        {
                            int temp = link->conditions[i].value.get_value<int>();
                            if (ImGui::DragInt("##trigger", &temp))
                                link->conditions[i].value = temp;
                            break;
                        }
                        case oo::Anim::P_TYPE::FLOAT:
                        {
                            float temp = link->conditions[i].value.get_value<float>();
                            if (ImGui::DragFloat("##trigger", &temp))
                                link->conditions[i].value = temp;
                            break;
                        }
                        }
                    }
                    ImGui::PopItemWidth();
                    ImGui::PopID();
                    ImGui::Separator();

                    if(requestDelete)
                        link->conditions.erase(link->conditions.begin() + i);
                }
            }
        }
    }

    if (ImGui::Button("+"))
    {
        ImGui::OpenPopup("##+");
    }
    if (ImGui::BeginPopup("##+"))
    {
        for (int i = 0; i < animator->GetActualComponent().animTree->parameters.size(); ++i)
        {
            if (ImGui::MenuItem(animator->GetActualComponent().animTree->parameters[i].name.c_str()))
            {
                switch (animator->GetActualComponent().animTree->parameters[i].type)
                {
                    case oo::Anim::P_TYPE::BOOL:
                    {
                        oo::Anim::ConditionInfo condition_info{
                            .comparison{oo::Anim::Condition::CompareType::EQUAL},
                            .parameter_name{animator->GetActualComponent().animTree->parameters[i].name},
                            .value{false}
                        };
                        auto& temp = animator->GetActualComponent().animTree->groups;
                        for (auto it = temp.begin(); it != temp.end(); ++it)
                            animator->AddCondition(it->second.name, link->name, condition_info);
                        break;
                    }
                    case oo::Anim::P_TYPE::TRIGGER:
                    {
                        oo::Anim::ConditionInfo condition_info{
                            .comparison{oo::Anim::Condition::CompareType::EQUAL},
                            .parameter_name{animator->GetActualComponent().animTree->parameters[i].name},
                            .value{false}
                        };
                        auto& temp = animator->GetActualComponent().animTree->groups;
                        for (auto it = temp.begin(); it != temp.end(); ++it)
                            animator->AddCondition(it->second.name, link->name, condition_info);
                        break;
                    }
                    case oo::Anim::P_TYPE::INT:
                    {
                        oo::Anim::ConditionInfo condition_info{
                            .comparison{oo::Anim::Condition::CompareType::GREATER},
                            .parameter_name{animator->GetActualComponent().animTree->parameters[i].name},
                            .value{0}
                        };
                        auto& temp = animator->GetActualComponent().animTree->groups;
                        for (auto it = temp.begin(); it != temp.end(); ++it)
                            animator->AddCondition(it->second.name, link->name, condition_info);
                        break;
                    }
                    case oo::Anim::P_TYPE::FLOAT:
                    {
                        oo::Anim::ConditionInfo condition_info{
                            .comparison{oo::Anim::Condition::CompareType::GREATER},
                            .parameter_name{animator->GetActualComponent().animTree->parameters[i].name},
                            .value{0.0f}
                        };
                        auto& temp = animator->GetActualComponent().animTree->groups;
                        for (auto it = temp.begin(); it != temp.end(); ++it)
                            animator->AddCondition(it->second.name, link->name, condition_info);
                        break;
                    }
                }
            }
        }
        ImGui::EndPopup();
    }
}

AnimatorControllerView::NodeInfo* AnimatorControllerView::CreateNode(oo::Anim::Node* _anim_node)
{
    m_nodes.emplace_back(_anim_node);
    m_nodes.back().Input.emplace_back(GetAvailableNodeID(), &m_nodes.back(), _anim_node->name.c_str());
    m_nodes.back().Output.emplace_back(GetAvailableNodeID(), &m_nodes.back(), _anim_node->name.c_str());
    m_nodes.back().pos = ImVec2(_anim_node->position.x, _anim_node->position.y);

    BuildNode(&m_nodes.back());

    return &m_nodes.back();
}

AnimatorControllerView::LinkInfo* AnimatorControllerView::CreateLink(oo::Anim::Link* link, ed::PinId inputPinId, ed::PinId outputPinId)
{
    m_links_.emplace_back( inputPinId, outputPinId, link);
    return &m_links_.back();
    /*auto& temp = _animator->GetActualComponent().animTree->groups;

    
    for (auto it = temp.begin(); it != temp.end(); ++it)
    {
        auto newLink = _animator->AddLink(it->second.name,
                                          FindNode(inputPinId)->anim_node->name,
                                          FindNode(outputPinId)->anim_node->name);
        m_links_.push_back({ ed::LinkId(m_nextLinkId++), inputPinId, outputPinId, newLink.operator->() });
    }
    return &m_links_.back();*/
}

void AnimatorControllerView::OnDelete()
{
    if (ed::BeginDelete())
    {
        ed::LinkId linkId = 0;
        while (ed::QueryDeletedLink(&linkId))
        {
            if (ed::AcceptDeletedItem())
            {
                auto id = std::find_if(m_links_.begin(), m_links_.end(), [linkId](auto& link) { return link.id == linkId; });
                if (id != m_links_.end())
                {
                    auto deletingLink = id->link;
                    animator->RemoveLink(oo::Anim::TargetLinkInfo{ .group_name{deletingLink->dst->group->name}, .link_ID{deletingLink->linkID} });
                    m_links_.erase(id);
                }
            }
        }

        ed::NodeId nodeId = 0;
        while (ed::QueryDeletedNode(&nodeId))
        {
            if (ed::AcceptDeletedItem())
            {
                auto id = std::find_if(m_nodes.begin(), m_nodes.end(), [nodeId](auto& node) { return node.id == nodeId; });
                if (id->anim_node->name == "Entry" || id->anim_node->name == "Exit" || id->anim_node->name == "Any State")
                    break;
                if (id != m_nodes.end())
                {
                    for (const auto& inputPin : id->Input)
                    {
                        for (size_t i = 0; i < m_links_.size(); i++)
                        {
                            if (m_links_[i].outputID == inputPin.id)
                            {
                                ed::DeleteLink(m_links_[i].id);
                                auto iter = std::find_if(m_links_.begin(), m_links_.end(), [&](const auto& link) { return link.id == m_links_[i].id; });
                                auto deletingLink = iter->link;
                                animator->RemoveLink(oo::Anim::TargetLinkInfo{ .group_name{deletingLink->dst->group->name}, .link_ID{deletingLink->linkID} });
                                m_links_.erase(iter);
                                continue;
                            }
                        }
                    }
                    for (const auto& outputPin : id->Output)
                    {
                        for (size_t i = 0; i < m_links_.size(); i++)
                        {
                            if (m_links_[i].inputID == outputPin.id)
                            {
                                ed::DeleteLink(m_links_[i].id);
                                auto iter = std::find_if(m_links_.begin(), m_links_.end(), [&](const auto& link) { return link.id == m_links_[i].id; });
                                auto deletingLink = iter->link;
                                animator->RemoveLink(oo::Anim::TargetLinkInfo{ .group_name{deletingLink->src->group->name}, .link_ID{deletingLink->linkID} });
                                m_links_.erase(iter);
                                continue;
                            }
                        }
                    }
                    auto deletingNodeContainer = FindNode(id->id);
                    auto deletingNode = deletingNodeContainer->anim_node;
                    animator->RemoveNode(oo::Anim::TargetNodeInfo{ .group_name{deletingNode->group->name}, .node_ID{deletingNode->node_ID} });
                    m_nodes.erase(id);
                }
            }
        }
    }
    ed::EndDelete();
}

void AnimatorControllerView::PostDisplayUpdate(oo::AnimationComponent* _animator)
{
    if (m_firstFrame == true) return;
    //update animator nodes
    for (auto& node : m_nodes)
    {
        node.pos = ed::GetNodePosition(node.id);
        if (node.pos.x == FLT_MAX)
            node.pos.x = 0.f;
        if (node.pos.y == FLT_MAX)
            node.pos.y = 0.f;
        //update animation node's position
        auto& position = node.anim_node->position;
        position.x = node.pos.x;
        position.y = node.pos.y;
    }

    
}

AnimatorControllerView::Pin* AnimatorControllerView::FindPin(ed::PinId id)
{
    if (!id)
        return nullptr;

    for (auto& node : m_nodes)
    {
        for (auto& pin : node.Input)
            if (pin.id == id)
                return &pin;

        for (auto& pin : node.Output)
            if (pin.id == id)
                return &pin;
    }

    return nullptr;
}

void AnimatorControllerView::LoadGraph(oo::AnimationComponent* _animator)
{
    if (m_firstFrame == false) return;
    if (_animator->HasAnimationTree() == false) return;
    if (_animator->GetActualComponent().animTree->groups.empty()) return;

    ed::SetCurrentEditor(m_Context);
    ed::Begin("Animator Controller Editor", ImVec2(0.0, 0.0));

    //initialize the node editor with data from animation tree
    auto& groups = _animator->GetActualComponent().animTree->groups;
    
    for (auto& [groupID, group] : groups)
    {
        current_group_name = group.name;

        for (auto& [nodeID, node] : group.nodes)
        {
            auto nodeinfo = CreateNode(&node);
            assert(nodeinfo);
        }

        for (auto& [linkID, link] : group.links)
        {
            NodeInfo* outputNode = FindNode(link.src.operator->());
            NodeInfo* inputNode = FindNode(link.dst.operator->());
            assert(outputNode && inputNode);
            if ((outputNode && inputNode) == false) continue;

            ed::PinId inputPin = outputNode->Output[0].id;
            ed::PinId outputPin = inputNode->Input[0].id;

            auto linkinfo = CreateLink(&link, inputPin, outputPin);

            ed::Link(linkinfo->id, linkinfo->inputID, linkinfo->outputID);
        }
    }
    
    ed::End();
    //no need to load data again after this
    m_firstFrame = false;
}

//void AnimatorControllerView::BuildNodes()
//{
//    for (int i = 0; i < m_nodes.size(); ++i)
//    {
//        //if (m_firstFrame)
//        BuildNode(m_nodes[i]);
//    }
//}

void AnimatorControllerView::BuildNodeOnEditor(NodeInfo& info)
{
    ed::SetNodePosition(info.id, info.pos);
    ed::BeginNode(info.id);
    ed::BeginPin(info.Input[0].id, ed::PinKind::Input);
    ImGui::Text("O");
    ed::EndPin();
    ImGui::SameLine();
    ImGui::Text(info.anim_node->name.c_str());
    ImGui::SameLine();
    ed::BeginPin(info.Output[0].id, ed::PinKind::Output);
    ImGui::Text(">");
    ed::EndPin();
    ed::EndNode();
}

void AnimatorControllerView::Clear()
{
    if (m_firstFrame == true) return;


    ed::SetCurrentEditor(m_Context);
    ed::Begin("Animator Controller Editor", ImVec2(0.0, 0.0));

    //return the ids for nodes
    for (auto& node : m_nodes)
    {
        ed::DeleteNode(node.id);
        
        //ReturnID(node.id);

        for (auto& pin : node.Input)
            ReturnID(pin.id);

        for (auto& pin : node.Output)
            ReturnID(pin.id);

    }

    //return the ids for links
    for (auto& link : m_links_)
    {
        ed::DeleteLink(link.id);

        //ReturnID(link.id);
    }
    //clear the containers for future use
    m_nodes.clear();
    m_links_.clear();

    //clear the delete queue
    if (ed::BeginDelete())
    {
        ed::LinkId linkId = 0;
        while (ed::QueryDeletedLink(&linkId))
        {
            if (ed::AcceptDeletedItem())
            {
            }
        }
        ed::NodeId nodeId = 0;
        while (ed::QueryDeletedNode(&nodeId))
        {
            if (ed::AcceptDeletedItem())
            {
            }
        }
    }
    ed::EndDelete();

    ed::End();

    //be ready to load data again
    m_firstFrame = true;
}
