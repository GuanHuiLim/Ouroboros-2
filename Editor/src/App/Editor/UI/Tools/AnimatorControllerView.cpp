#include "pch.h"
#include "AnimatorControllerView.h"
#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/Scene/Scene.h>
#include "App/Editor/UI/Object Editor/Hierarchy.h"

#include "App/Editor/Utility/ImGuiManager.h"

AnimatorControllerView::NodeInfo* AnimatorControllerView::FindNode(ed::NodeId id)
{
    for (auto& node : m_nodes)
        if (node.id == id)
            return &node;

    return nullptr;
}

AnimatorControllerView::NodeInfo* AnimatorControllerView::FindNode(oo::Anim::Node* _node)
{
    for (auto& node : m_nodes)
        if (node.anim_node == _node)
            return &node;
    return nullptr;
}

void AnimatorControllerView::Show()
{
    //if(animator), DisplayAnimatorController();

    //Get Info From the GameObject
    auto& selected_items = Hierarchy::GetSelected();
    auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();

    if (selected_items.size() <= 0)
        return;

    auto gameobject = scene->FindWithInstanceID(*selected_items.begin());

    if (gameobject->HasComponent<oo::AnimationComponent>() == false)
        return;

    animator = &gameobject.get()->GetComponent<oo::AnimationComponent>();

    DisplayAnimatorController(animator);
}

void AnimatorControllerView::BuildNode(NodeInfo* node)
{
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
}

void AnimatorControllerView::DisplayAnimatorController(oo::AnimationComponent* _animator)
{
    ed::SetCurrentEditor(m_Context);
    ed::Begin("Animator Controller Editor", ImVec2(0.0, 0.0));

    if (!_animator)
    {
        ed::End();
        return;
    }

    //Handle for everytime i press a new animatortree
    if (m_firstFrame)
    {
        //initialize the node editor with data from animation tree
        for (int i = 0; i < _animator->GetActualComponent().animTree->groups[0].nodes.size(); ++i)
        {
            CreateNode(uniqueId, &(_animator->GetActualComponent().animTree->groups[0].nodes[i]));
        }
    }

    //
    // 1) Commit known data to editor
    //

    for (int i = 0; i < m_nodes.size(); ++i)
    {
        if (m_firstFrame)
            ed::SetNodePosition(m_nodes[i].id, m_nodes[i].pos);
        ed::BeginNode(m_nodes[i].id);
        ed::BeginPin(m_nodes[i].Input[0].id, ed::PinKind::Input);
        ImGui::Text("O");
        ed::EndPin();
        ImGui::SameLine();
        ImGui::Text(m_nodes[i].anim_node->name.c_str());
        ImGui::SameLine();
        ed::BeginPin(m_nodes[i].Output[0].id, ed::PinKind::Output);
        ImGui::Text(">");
        ed::EndPin();
        ed::EndNode();
    }

    // Submit Links
    for (auto& linkInfo : m_links)
    {
        ed::Link(linkInfo.id, linkInfo.inputID, linkInfo.outputID);
    }
    for (auto& LinkInfo : m_links_)
    {
        ed::Link(LinkInfo.id, LinkInfo.inputID, LinkInfo.outputID);
    }

    if (m_firstFrame)
    {
        for (int i = 0; i < _animator->GetActualComponent().animTree->groups[0].links.size(); ++i)
        {
            NodeInfo* outputNode = FindNode(&_animator->GetActualComponent().animTree->groups[0].links[i].src);
            NodeInfo* inputNode = FindNode(&_animator->GetActualComponent().animTree->groups[0].links[i].dst);

            ed::PinId inputPin = outputNode->Output[0].id;
            ed::PinId outputPin = inputNode->Input[0].id;

            //m_links.push_back(LinkInfo(ed::LinkId(m_nextLinkId++), inputPin, outputPin, &(_animator->groups[0].links[i])));
            m_links_.push_back(LinkInfo(ed::LinkId(m_nextLinkId++), inputPin, outputPin));
            m_links_.back().link = &(_animator->GetActualComponent().animTree->groups[0].links[i]);
            ed::Link(m_links_.back().id, m_links_.back().inputID, m_links_.back().outputID);
        }
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
                    // Since we accepted new link, lets add one to our list of links.
                    for (int i = 0; i < _animator->GetActualComponent().animTree->groups[0].links.size(); ++i)
                    {
                        m_links_.push_back({ ed::LinkId(m_nextLinkId++), inputPinId, outputPinId });
                        m_links_.back().link = &(_animator->GetActualComponent().animTree->groups[0].links[i]);
                        m_links.push_back({ ed::LinkId(m_nextLinkId), inputPinId, outputPinId });
                        m_links.back().link = &(_animator->GetActualComponent().animTree->groups[0].links[i]);
                    }

                    // Draw new link.
                    ed::Link(m_links_.back().id, m_links_.back().inputID, m_links_.back().outputID);
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
            //oo::Anim::NodeInfo nodeinfo{
            //.name{ "Node" },
            //.animation_name{ oo::Anim::Animation::empty_animation_name },
            //.speed{ 1.f },
            //.position{0.f,0.f,0.f},
            //};

            //oo::Anim::Node* temp = _animator->AddNode(_animator->GetActualComponent().animTree->groups[0].name, nodeinfo);

            //CreateNode(uniqueId, temp);
        }
        ImGui::EndPopup();
    }
    ed::Resume();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
        for (auto& link : m_links)
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
    if (ImGui::Begin("Parameters"))
    {
        ImVec2 textsize = ImGui::CalcTextSize("a");

        for (int i = 0; i < animator->GetActualComponent().animTree->parameters.size(); ++i)
        {
            if (animator->GetActualComponent().animTree->parameters[i].value.is_type<bool>())
            {
                ImGui::Text(animator->GetActualComponent().animTree->parameters[i].name.c_str());
                ImGui::SameLine(textsize.x * 15);
                std::string tag = "##" + animator->GetActualComponent().animTree->parameters[i].name;
                bool& temp = animator->GetActualComponent().animTree->parameters[i].value.get_value<bool>();
                ImGui::Checkbox(tag.c_str(), &temp);
            }
            else if (animator->GetActualComponent().animTree->parameters[i].value.is_type<float>())
            {
                ImGui::Text(animator->GetActualComponent().animTree->parameters[i].name.c_str());
                ImGui::SameLine(textsize.x * 15);
                std::string tag = "##" + animator->GetActualComponent().animTree->parameters[i].name;
                float& temp = animator->GetActualComponent().animTree->parameters[i].value.get_value<float>();
                ImGui::InputFloat(tag.c_str(), &temp);
            }
            else if (animator->GetActualComponent().animTree->parameters[i].value.is_type<int>())
            {
                ImGui::Text(animator->GetActualComponent().animTree->parameters[i].name.c_str());
                ImGui::SameLine(textsize.x * 15);
                std::string tag = "##" + animator->GetActualComponent().animTree->parameters[i].name;
                int& temp = animator->GetActualComponent().animTree->parameters[i].value.get_value<int>();
                ImGui::InputInt(tag.c_str(), &temp);
            }
        }

        ImGui::End();
    }
}

void AnimatorControllerView::DisplayInspector()
{
    m_nodesId.resize(ed::GetSelectedObjectCount());
    m_linksId.resize(ed::GetSelectedObjectCount());

    int nodeCount = ed::GetSelectedNodes(m_nodesId.data(), static_cast<int>(m_nodesId.size()));
    int linkCount = ed::GetSelectedLinks(m_linksId.data(), static_cast<int>(m_linksId.size()));

    m_nodesId.resize(nodeCount);
    m_linksId.resize(linkCount);

    if (ImGui::Begin("Animator Inspector"))
    {
        if (nodeCount != 0)
        {
            for (int i = 0; i < nodeCount; ++i)
            {
                ed::NodeId temp = m_nodesId[i];
                auto id = std::find_if(m_nodes.begin(), m_nodes.end(), [temp](auto& node) { return node.id == temp; });
                ImGui::Text("Name");
                ImVec2 textsize = ImGui::CalcTextSize("a");
                ImGui::SameLine(textsize.x * 8);
                ImGui::InputText("##name", const_cast<char*>(id->anim_node->name.c_str()), 256);
                ImGui::Separator();
                ImGui::Text("Animation");
                ImGui::SameLine(textsize.x * 12);
                ImGui::InputText("##animation", const_cast<char*>(id->anim_node->GetAnimation().name.c_str()), 256);
                ImGui::Text("Speed");
                ImGui::SameLine(textsize.x * 12);
                ImGui::InputFloat("##speed", &id->anim_node->speed);
            }
        }
        else if (linkCount != 0)
        {
            for (int i = 0; i < linkCount; ++i)
            {
                ed::LinkId temp = m_linksId[i];
                auto id = std::find_if(m_links_.begin(), m_links_.end(), [temp](auto& link) {return link.id == temp; });
                std::string linkRelation = id->link->name;
                ImGui::Text(linkRelation.c_str());
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
                        //ImGui::Text("Transition Duration");
                        //ImGui::SameLine(textsize.x * 25);
                        //ImGui::InputFloat("##transitionDuration", &transitionDuration);
                    }
                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();
    }
}

AnimatorControllerView::NodeInfo* AnimatorControllerView::CreateNode(int& uniqueId, oo::Anim::Node* _anim_node)
{
    m_nodes.emplace_back(uniqueId++, _anim_node);
    m_nodes.back().Input.emplace_back(uniqueId++, &m_nodes.back(), _anim_node->name.c_str());
    m_nodes.back().Output.emplace_back(uniqueId++, &m_nodes.back(), _anim_node->name.c_str());
    m_nodes.back().pos = ImVec2(_anim_node->position.x, _anim_node->position.y);

    BuildNode(&m_nodes.back());

    return &m_nodes.back();
}

AnimatorControllerView::LinkInfo* AnimatorControllerView::CreateLink(int& uniqueId, oo::Anim::Link* _anim_link)
{
    

    return nullptr;
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
                    m_links_.erase(id);
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
                        for (size_t i = 0; i < m_links.size(); i++)
                        {
                            if (m_links[i].outputID == inputPin.id)
                            {
                                ed::DeleteLink(m_links[i].id);
                                auto iter = std::find_if(m_links.begin(), m_links.end(), [&](const auto& link) { return link.id == m_links[i].id; });
                                m_links.erase(iter);
                                continue;
                            }
                        }
                    }
                    for (const auto& outputPin : id->Output)
                    {
                        for (size_t i = 0; i < m_links.size(); i++)
                        {
                            if (m_links[i].inputID == outputPin.id)
                            {
                                ed::DeleteLink(m_links[i].id);
                                auto iter = std::find_if(m_links.begin(), m_links.end(), [&](const auto& link) { return link.id == m_links[i].id; });
                                m_links.erase(iter);
                                continue;
                            }
                        }
                    }
                    m_nodes.erase(id);
                }
            }
        }
    }
    ed::EndDelete();
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
