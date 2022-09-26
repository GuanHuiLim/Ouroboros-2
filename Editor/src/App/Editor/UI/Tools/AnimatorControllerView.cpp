#include "pch.h"
#include "AnimatorControllerView.h"

void AnimatorControllerView::Show()
{
    static bool displayAnimatorInspector = true;
    static LinkInfo* selectedLink = nullptr;
    static ed::NodeId* selectedNode = nullptr;

    ed::SetCurrentEditor(m_Context);
    ed::Begin("Animator Controller Editor", ImVec2(0.0, 0.0));

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
        ImGui::Text(m_nodes[i].name.c_str());
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
                    m_links.push_back({ ed::LinkId(m_nextLinkId++), inputPinId, outputPinId });

                    // Draw new link.
                    ed::Link(m_links.back().id, m_links.back().inputID, m_links.back().outputID);
                }

                // You may choose to reject connection between these nodes
                // by calling ed::RejectNewItem(). This will allow editor to give
                // visual feedback by changing link thickness and color.
            }
        }
    }
    ed::EndCreate(); // Wraps up object creation action handling.

    for (auto& linkInfo : m_links)
    {
        // if linkid is selected
        if (ed::IsLinkSelected(linkInfo.id))
        {
            // store the LinkId of the LinkInfo into a temp LinkId
            //      LinkId selectedLinkID = linkInfo.Id;
            selectedNode = nullptr;
            selectedLink = &linkInfo;
        }
    }

    for (int i = 0; i < m_nodes.size(); ++i)
    {
        if (ed::IsNodeSelected(m_nodes[i].id))
        {
            selectedLink = nullptr;
            selectedNode = &m_nodes[i].id;
        }
    }

    if (ImGui::IsWindowFocused() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        selectedNode = nullptr;
        selectedLink = nullptr;
    }

    // Handle deletion action
    OnDelete();

    // End of interaction with editor.

    //if selectedNode or selectedLink is !null, display info of the selected item

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
            CreateNode(uniqueId, "State", ImVec2(0,0));
        }
        ImGui::EndPopup();
    }
    ed::Resume();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
        for (auto& link : m_links)
            ed::Flow(link.id);

    ed::End();

    //MUST DO THIS TO MOVE THE NODES
    m_firstFrame = false;

    DisplayInspector(displayAnimatorInspector, selectedLink, selectedNode);
    DisplayParameters();
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

void AnimatorControllerView::DisplayParameters()
{
    if (ImGui::Begin("Parameters"))
    {
        ImGui::End();
    }
}

void AnimatorControllerView::DisplayInspector(bool& displayAnimatorInspector, LinkInfo* selectedLink, ed::NodeId* selectedNode)
{
    if (ImGui::Begin("Animator Inspector", &displayAnimatorInspector))
    {
        if (selectedLink)
        {
            ed::LinkId temp = selectedLink->id;
            auto id = std::find_if(m_links.begin(), m_links.end(), [temp](auto& link) {return link.id == temp; });

            Pin* input = FindPin(selectedLink->inputID);
            Pin* output = FindPin(selectedLink->outputID);

            std::string linkRelation = input->name + "->" + output->name;
            ImGui::Text(linkRelation.c_str());
            static bool hasExitTime = false;
            static float exitTime = 0.75f;
            static bool fixedDuration = true;
            static float transitionDuration = 0.1f;
            ImVec2 textsize = ImGui::CalcTextSize("a");
            ImGui::Text("Has Exit Time");
            ImGui::SameLine(textsize.x * 25);
            ImGui::Checkbox("##hasexittime", &hasExitTime);
            if (ImGui::TreeNode("Settings"))
            {
                {
                    ImGui::Text("Exit Time");
                    ImGui::SameLine(textsize.x * 25);
                    if (hasExitTime)
                        ImGui::InputFloat("##exitTime", &exitTime, 0.0f, 0.0f, "%.2f");
                    else
                        ImGui::InputFloat("##exitTime", &exitTime, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_ReadOnly);
                    ImGui::Text("Fixed Duration");
                    ImGui::SameLine(textsize.x * 25);
                    ImGui::Checkbox("##fixedduration", &fixedDuration);
                    ImGui::Text("Transition Duration");
                    ImGui::SameLine(textsize.x * 25);
                    ImGui::InputFloat("##transitionDuration", &transitionDuration);
                }
                ImGui::TreePop();
            }
        }
        else if (selectedNode)
        {
            ed::NodeId temp = *selectedNode;
            auto id = std::find_if(m_nodes.begin(), m_nodes.end(), [temp](auto& node) {return node.id == temp; });
            static char animation[256];
            static float speed = 1;
            ImGui::Text("Name");
            ImVec2 textsize = ImGui::CalcTextSize("a");
            ImGui::SameLine(textsize.x * 15);
            ImGui::InputText("##name", const_cast<char*>(id->name.c_str()), 256);
            ImGui::Separator();
            ImGui::Text("Animation");
            ImGui::SameLine(textsize.x * 15);
            ImGui::InputText("##animation", animation, 256);
            ImGui::Text("Speed");
            ImGui::SameLine(textsize.x * 15);
            ImGui::InputFloat("##speed", &speed);
        }
        ImGui::End();
    }
}

AnimatorControllerView::NodeInfo* AnimatorControllerView::CreateNode(int& uniqueId, const char* _name, ImVec2 _pos)
{
    m_nodes.emplace_back(uniqueId++, _name);
    m_nodes.back().Input.emplace_back(uniqueId++, &m_nodes.back(), _name, PinType::Flow);
    m_nodes.back().Output.emplace_back(uniqueId++, &m_nodes.back(), _name, PinType::Flow);
    m_nodes.back().pos = _pos;

    BuildNode(&m_nodes.back());

    return &m_nodes.back();
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
                auto id = std::find_if(m_links.begin(), m_links.end(), [linkId](auto& link) { return link.id == linkId; });
                if (id != m_links.end())
                    m_links.erase(id);
            }
        }

        ed::NodeId nodeId = 0;
        while (ed::QueryDeletedNode(&nodeId))
        {
            if (ed::AcceptDeletedItem())
            {
                auto id = std::find_if(m_nodes.begin(), m_nodes.end(), [nodeId](auto& node) { return node.id == nodeId; });
                if (id->name == "Entry" || id->name == "Exit" || id->name == "Any State")
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
