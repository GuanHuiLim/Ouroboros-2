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

//ASSET MANAGER EXAMPLE
//static oo::AssetManager manager = oo::AssetManager("./assets");
//static oo::Asset myImageAsset;

void EditorLayer::OnAttach()
{
    ImGuiManager_Launcher::Create("project tracker", true, ImGuiWindowFlags_None, [this]() { this->m_tracker.Show(); });
#ifdef OO_EDITOR
	ImGuiManager::InitAssetsAll();
#endif

	//ASSET MANAGER EXAMPLE
	/*try
	{
		//Asset myFile = manager.LoadFile("assets/infile.txt");
		manager.LoadDirectory("./");
		auto v = manager.LoadName("Arcadia.png", false);
		if (v.size() > 0)
			myImageAsset = v[0];
	}
	catch (...)
	{
		std::cout << "not found\n";
	}*/
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

	//ASSET MANAGER EXAMPLE
	/*if (myImageAsset.HasData())
	{
		ImGui::Image(reinterpret_cast<void*>(myImageAsset.GetData<ImTextureID>()), ImVec2(100, 100));
	}*/

	//top menu bar
	//Editor::MenuBar();
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			if (ImGui::MenuItem("Open Launcher")) 
			{
#ifdef OO_EDITOR
				CloseProjectEvent e;
				OpenPromptEvent<CloseProjectEvent> ope(e, [this]() {this->SetEditorMode(false); });
				oo::EventManager::Broadcast(&ope);
#else
				m_editormode = false;
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
