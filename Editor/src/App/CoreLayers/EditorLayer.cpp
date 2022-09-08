/************************************************************************************//*!
\file           EditorLayer.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 31, 2022
\brief          Defines a layer that will be running during editor mode
                and its related events

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "EditorLayer.h"

#include <Ouroboros/Core/Input.h>

// Project Tracker related includes
#include <Launcher/Utilities/ImGuiManager_Launcher.h>
#include <Ouroboros/EventSystem/EventManager.h>
#include <App/Editor/Events/LoadProjectEvents.h>

#include <Ouroboros/Core/Application.h>
#include <Ouroboros/Vulkan/VulkanContext.h>




void EditorLayer::OnAttach()
{
    ImGuiManager_Launcher::Create("project tracker", true, ImGuiWindowFlags_None, [this]() { this->m_tracker.Show(); });
#ifdef OO_EDITOR
	ImGuiManager::InitAssetsAll();
#endif
}

// TODO : IMGUI DOESNT WORK YET FOR NOW. VULKAN NEEDS TO BE SET UP
// PROPERLY FOR IMGUI RENDERING TO TAKE PLACE

void EditorLayer::OnUpdate()
{
#ifndef OO_END_PRODUCT
    if(m_editormode == false)
        ImGuiManager_Launcher::UpdateAllUI();
#endif
#ifdef OO_EDITOR
    else
	    m_editor.Update();
#endif


    /*if (myImageAsset.HasData())
    {
        auto num = myImageAsset.GetData<uint32_t>();
        auto vc = reinterpret_cast<oo::VulkanContext*>(oo::Application::Get().GetWindow().GetRenderingContext());
        auto vr = vc->getRenderer();
        ImGui::Image(reinterpret_cast<void*>(vr->GetImguiID(num)), ImVec2(100, 100));
    }*/
	//std::cout << "loaded image data is " << *myImageAsset.GetData<uint32_t>() << '\n';

	//top menu bar
	//Editor::MenuBar();
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			if (ImGui::MenuItem("Open Launcher")) 
			{
				m_editormode = false;
#ifdef OO_EDITOR
				CloseProjectEvent e;
				oo::EventManager::Broadcast(&e);
#endif
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
    //m_editor.ShowAllWidgets();

    //if (m_demo)
    //    ImGui::ShowDemoWindow(&m_demo);


}
