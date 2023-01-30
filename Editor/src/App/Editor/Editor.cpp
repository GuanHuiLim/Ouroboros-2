/************************************************************************************//*!
\file           Editor.cpp
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          start of most editor code 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Editor.h"
#include "Project.h"
#include "Serializer.h"
#include "Utility/ImGuiManager.h"
#include "App/Editor/Utility/ImGui_PopupId.h"

#include "App/Editor/UI/Tools/WarningMessage.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

#include "Ouroboros/Commands/CommandStackManager.h"
#include "SceneManagement/include/SceneManager.h"

#include <Ouroboros/EventSystem/EventManager.h>
#include "App/Editor/Events/LoadSceneEvent.h"
#include "App/Editor/Events/ImGuiRestartEvent.h"
#include "App/Editor/Events/OpenFileEvent.h"
#include "App/Editor/Events/OpenPromtEvent.h"
#include "App/Editor/Events/CopyButtonEvent.h"
#include "App/Editor/Events/PasteButtonEvent.h"
#include "App/Editor/Events/DuplicateButtonEvent.h"
#include "App/Editor/Events/DestroyGameObjectButtonEvent.h"

#include "Ouroboros/Core/Events/FileDropEvent.h"
#include "Ouroboros/Core/Timer.h"
#include <Ouroboros/Physics/PhysicsSystem.h>
#include <Ouroboros/Vulkan/RendererSystem.h>
#include <Ouroboros/Scene/RuntimeController.h>
static void FileDrop(oo::FileDropEvent* e)
{
	static std::set<std::string> s{ ".png", ".jpg", ".jpeg", ".ogg" ,".ogg", ".mp3", ".wav" ,".fbx",".FBX",".ttf", ".otf", ".tga"};
	if (e->GetType() != oo::FileDropType::DropFile)
		return;
	std::filesystem::path p = e->GetFile();
	std::string ext = p.extension().string();
	if (ext == ".prefab")
	{
		std::filesystem::copy(p, Project::GetPrefabFolder() / p.filename(), std::filesystem::copy_options::overwrite_existing);
	}
	else if (ext == ".scn")
	{
		std::filesystem::copy(p, Project::GetSceneFolder() / p.filename(), std::filesystem::copy_options::overwrite_existing);
	}
	else if (ext == ".cs")
	{
		//std::filesystem::copy(p, Project::GetScriptBuildPath() / p.filename(), std::filesystem::copy_options::overwrite_existing);
	}
	else
	{
		auto iter = s.find(ext);
		if (iter == s.end())
		{
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, "Engine don't support this type");
			return;
		}
		std::filesystem::copy(p, Project::GetAssetFolder() / p.filename(),std::filesystem::copy_options::overwrite_existing);
		Project::GetAssetManager()->Scan();
		NetworkingFileTransferEvent nfte;

		nfte.p = std::filesystem::relative(Project::GetAssetFolder() / p.filename(), Project::GetProjectFolder());
		oo::EventManager::Broadcast<NetworkingFileTransferEvent>(&nfte);
	}
}
Editor::Editor()
{
	oo::CommandStackManager::InitEvents();
	oo::EventManager::Subscribe<oo::FileDropEvent>(&FileDrop);

	AddSequence(TimedSequence([] {
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		Serializer::SaveScene(*(scene));
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Auto Saved");
		}, 240.0f));
	
	//object editors
	ImGuiManager::Create("Hierarchy", true, ImGuiWindowFlags_MenuBar, [this] {this->m_hierarchy.Show(); }, 0, false);
	ImGuiManager::Create("Inspector", true, ImGuiWindowFlags_MenuBar, [this] {this->m_inspector.Show(); }, 0, false);
	ImGuiManager::Create("FileBrowser", true, ImGuiWindowFlags_MenuBar, [this] {this->m_fileBrowser.Show(); }, 0, false);
	ImGuiManager::Create("Editor Viewport", true, (ImGuiWindowFlags_)(ImGuiWindowFlags_NoBackground |ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar), [this] {this->m_EditorViewport.Show(); }, [this] {this->m_EditorViewport.UpdateWhenNotShown(); }, false);
	ImGuiManager::Create("Preview Window", true, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollWithMouse), [this] {this->m_previewWindow.Show(); }, [this] {this->m_previewWindow.UpdateWhenNotShown(); }, false);
	ImGuiManager::Create("Script Sequencer", false, ImGuiWindowFlags_None, [this] {this->m_scriptSequencer.Show(); });

	//tools
	ImGuiManager::Create("Style Editor", false, ImGuiWindowFlags_MenuBar, [this] {this->m_styleEditor.Show(); });
	ImGuiManager::Create("PenTool", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_NoDecoration), [this] {this->m_pentool.Show(); });
	ImGuiManager::Create("Toolbar", true, ImGuiWindowFlags_None, [this] {this->m_toolbar.Show(); }, 0, false);
	ImGuiManager::Create("Logger", true, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar), [this] {this->m_loggingView.Show(); }, 0, false);
	ImGuiManager::Create("Animator Controller", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar), [this] {this->m_animatorControllerView.Show(); });
	ImGuiManager::Create("Animation Timeline", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar), [this] {this->m_animationTimelineView.Show(); });
	ImGuiManager::Create("Mesh Hierarchy", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar ), [this] {this->m_meshHierarchy.Show(); });
	//ImGuiManager::Create("Renderer Debugger", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar), [this] {this->m_rendererDebugger.Show(); });
	ImGuiManager::Create("Script Sequencer", true, ImGuiWindowFlags_None, [this] {this->m_scriptSequencer.Show(); });

	//external (project based tools)
	ImGuiManager::Create("Scene Manager", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar), [this] {this->m_sceneOderingWindow.Show(); });
	ImGuiManager::Create("Input Manager", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar), [this] {this->m_inputManager.Show(); });
	ImGuiManager::Create("Renderer Fields", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar), [this] {this->m_rendererFieldsWindow.Show(); });


	//ImGuiManager::Create("##helper", true, ImGuiWindowFlags_None, [this] {this->helper.Popups(); });
}

Editor::~Editor()
{
	//commands only exist on editor so it makes sense that the editor handles the destruction of it
	oo::CommandStackManager::ClearCommandBuffer();
}

void Editor::Update()
{
	static bool b = [this]() 
	{
		//ImGuiManager::InitAssetsAll();
		m_styleEditor.InitStyle(); 
		m_toolbar.InitAssets();
		m_loggingView.InitAsset();
		m_fileBrowser.InitAssets();
		return true; 
	}();
	ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	MenuBar();

	ImGuiManager::UpdateAllUI();
	m_warningMessage.Show();
	m_chatsystem.Show();

	m_Keylogger.Show();
	helper.Popups();
	

	if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftCtrl))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_S))
		{
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			Serializer::SaveScene(*(scene));
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_Z))
		{
			oo::CommandStackManager::UndoCommand();
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_Y))
		{
			oo::CommandStackManager::RedoCommand();
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_C))
		{
			CopyButtonEvent cbe;
			oo::EventManager::Broadcast<CopyButtonEvent>(&cbe);
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_V))
		{
			PasteButtonEvent cbe;
			oo::EventManager::Broadcast<PasteButtonEvent>(&cbe);
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_D))
		{
			DuplicateButtonEvent dbe;
			oo::EventManager::Broadcast<DuplicateButtonEvent>(&dbe);
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_P))
		{
			if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftShift))
			{
				ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::PAUSE);
				oo::EventManager::Broadcast(&tbe);
			}
			else if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftAlt))
			{
				ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::STOP);
				oo::EventManager::Broadcast(&tbe);
			}
			else
			{
				ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::PLAY);
				oo::EventManager::Broadcast(&tbe);
			}
		}
	}
	if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_Delete))
	{
		DestroyGameObjectButtonEvent dbe;
		oo::EventManager::Broadcast<DestroyGameObjectButtonEvent>(&dbe);
	}
	TimedUpdate();
}

void Editor::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Reset ImGui"))
			{
				ImGuiRestartEvent restartEvent;
				oo::EventManager::Broadcast(&restartEvent);
			}
			if (ImGui::MenuItem(oo::OO_TracyProfiler::m_server_active ? ("Close Profiler") : ("Open Profiler")))
			{
				if (oo::OO_TracyProfiler::m_server_active)
					oo::OO_TracyProfiler::CloseTracyServer();
				else
					oo::OO_TracyProfiler::StartTracyServer();
			}
			if (ImGui::MenuItem("Style Editor", nullptr, ImGuiManager::GetItem("Style Editor").m_enabled))
			{
				ImGuiManager::GetItem("Style Editor").m_enabled = !ImGuiManager::GetItem("Style Editor").m_enabled;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Windows"))
		{
			if (ImGui::MenuItem("Scene Manager",0, ImGuiManager::GetItem("Scene Manager").m_enabled))
			{
				ImGuiManager::GetItem("Scene Manager").m_enabled = !ImGuiManager::GetItem("Scene Manager").m_enabled;
			}
			if (ImGui::MenuItem("Input Manager",0, ImGuiManager::GetItem("Input Manager").m_enabled))
			{
				ImGuiManager::GetItem("Input Manager").m_enabled = !ImGuiManager::GetItem("Input Manager").m_enabled;
			}
			if (ImGui::MenuItem("Script Sequencer", 0, ImGuiManager::GetItem("Script Sequencer").m_enabled))
			{
				ImGuiManager::GetItem("Script Sequencer").m_enabled = !ImGuiManager::GetItem("Script Sequencer").m_enabled;
			}
			if (ImGui::MenuItem("Renderer Fields", 0, ImGuiManager::GetItem("Renderer Fields").m_enabled))
			{
				ImGuiManager::GetItem("Renderer Fields").m_enabled = !ImGuiManager::GetItem("Renderer Fields").m_enabled;
			}
			if (ImGui::BeginMenu("Animation"))
			{
				if (ImGui::MenuItem("Animation Timeline", 0, ImGuiManager::GetItem("Animation Timeline").m_enabled))
				{
					ImGuiManager::GetItem("Animation Timeline").m_enabled = !ImGuiManager::GetItem("Animation Timeline").m_enabled;
				}
				if (ImGui::MenuItem("Animator Controller", 0, ImGuiManager::GetItem("Animator Controller").m_enabled))
				{
					ImGuiManager::GetItem("Animator Controller").m_enabled = !ImGuiManager::GetItem("Animator Controller").m_enabled;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debugging"))
		{
			if (ImGui::MenuItem("Colliders Debug Draw", 0, oo::PhysicsSystem::ColliderDebugDraw))
			{
				oo::PhysicsSystem::ColliderDebugDraw = !oo::PhysicsSystem::ColliderDebugDraw;
			}
			if (ImGui::MenuItem("Physics Debug Messages", 0, oo::PhysicsSystem::DebugMessages))
			{
				oo::PhysicsSystem::DebugMessages = !oo::PhysicsSystem::DebugMessages;
			}
			if (ImGui::MenuItem("Camera Debug Draw", 0, oo::RendererSystem::CameraDebugDraw))
			{
				oo::RendererSystem::CameraDebugDraw = !oo::RendererSystem::CameraDebugDraw;
			}
			if (ImGui::MenuItem("Lights Debug Draw", 0, oo::RendererSystem::LightsDebugDraw))
			{
				oo::RendererSystem::LightsDebugDraw = !oo::RendererSystem::LightsDebugDraw;
			}
			
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Compile Scripts"))
		{
			//this is a legacy event but rn im too lazy to make a new event and rename everything
			ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::COMPILE);
			oo::EventManager::Broadcast(&tbe);
		}
		ImGui::EndMainMenuBar();
	}
}

void Editor::AddSequence(TimedSequence&& seq)
{
	m_timedseq.push_back(seq);
}

void Editor::TimedUpdate()
{
	float dt = oo::timer::dt();
	for (auto& curr_seq : m_timedseq)
	{
		curr_seq.curr_duration -= dt;
		if (curr_seq.curr_duration <= 0)
		{
			curr_seq.instruction();
			curr_seq.curr_duration = curr_seq.max_duration;
		}
	}
}

PopupHelperWindow::PopupHelperWindow()
{
	oo::EventManager::Subscribe<PopupHelperWindow, OpenPromptEvent<CloseProjectEvent>>(this, &PopupHelperWindow::CloseProjectEvent_EventReceiver);
	oo::EventManager::Subscribe<PopupHelperWindow, OpenPromptEvent<OpenFileEvent>>(this, &PopupHelperWindow::OpenFileEvent_EventReceiver);
}

void PopupHelperWindow::Popups()
{
	CloseProjectPopup();
	OpenFilePopup();
}

void PopupHelperWindow::CloseProjectEvent_EventReceiver(OpenPromptEvent<CloseProjectEvent>* e)
{
	if (ImGui::IsPopupOpen(ImGuiID{ 0 }, ImGuiPopupFlags_::ImGuiPopupFlags_AnyPopup))
		return;

	eventAfterPrompt = e->nextAction;
	closeproject = true;
}

void PopupHelperWindow::OpenFileEvent_EventReceiver(OpenPromptEvent<OpenFileEvent>* e)
{
	if (e->nextAction.nextEvent.m_type == OpenFileEvent::FileType::SCENE || 
		e->nextAction.nextEvent.m_type == OpenFileEvent::FileType::PREFAB)
	{
		if (ImGui::IsPopupOpen(ImGuiID{ 0 }, ImGuiPopupFlags_::ImGuiPopupFlags_AnyPopup))
			return;
		eventAfterPrompt_ofe = e->nextAction;
		openfile = true;
		return;
	}

	oo::EventManager::Broadcast<OpenFileEvent>(&e->nextAction.nextEvent);
	return;

}

void PopupHelperWindow::CloseProjectPopup()
{
	if (closeproject)
	{
		ImGui::OpenPopup("CloseProjectPrompt");
		closeproject = false;
	}
	if (ImGui::BeginPopupModal("CloseProjectPrompt", 0, ImGuiWindowFlags_NoDecoration))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,10.0f });
		ImVec2 txtsize = ImGui::CalcTextSize("Do You Want To Save Your Project?");
		ImGui::Dummy({ (ImGui::GetContentRegionAvail().x - txtsize.x) * 0.5f ,0 });
		ImGui::SameLine();
		ImGui::Text("Do You Want To Save Your Project?");
		constexpr float buttonsizeX = 50.0f;
		float paddingX = (ImGui::GetContentRegionAvail().x - buttonsizeX * 3) / 4;
		ImGui::Dummy({ paddingX, 0 }); ImGui::SameLine();
		if (ImGui::Button("Yes", { buttonsizeX,0 }))
		{
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			Serializer::SaveScene(*(scene));

			oo::EventManager::Broadcast(&eventAfterPrompt.nextEvent);
			if (eventAfterPrompt.nextAction)
				eventAfterPrompt.nextAction();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		ImGui::Dummy({ paddingX, 0 }); ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0, 0, 1.0f));
		if (ImGui::Button("No", { buttonsizeX,0 }))
		{
			oo::EventManager::Broadcast(&eventAfterPrompt.nextEvent);
			if (eventAfterPrompt.nextAction)
				eventAfterPrompt.nextAction();
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor(1);

		ImGui::SameLine();
		ImGui::Dummy({ paddingX, 0 }); ImGui::SameLine();
		if (ImGui::Button("Cancel", { buttonsizeX,0 }))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
}

void PopupHelperWindow::OpenFilePopup()
{
	if (openfile)
	{
		ImGui::OpenPopup("OpenFilePopup");
		openfile = false;
	}
	if (ImGui::BeginPopupModal("OpenFilePopup", 0, ImGuiWindowFlags_NoDecoration))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,10.0f });
		ImVec2 txtsize = ImGui::CalcTextSize("Do You Want To Save Your Scene?");
		ImGui::Dummy({ (ImGui::GetContentRegionAvail().x - txtsize.x) * 0.5f ,0 });
		ImGui::SameLine();
		ImGui::Text("Do You Want To Save Your Scene?");
		constexpr float buttonsizeX = 50.0f;
		float paddingX = (ImGui::GetContentRegionAvail().x - buttonsizeX * 3) / 4;
		ImGui::Dummy({ paddingX, 0 }); ImGui::SameLine();
		if (ImGui::Button("Yes", { buttonsizeX,0 }))
		{
			//save the scene before the scene changes
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			Serializer::SaveScene(*(scene));
			
			oo::EventManager::Broadcast(&eventAfterPrompt_ofe.nextEvent);
			if (eventAfterPrompt_ofe.nextAction)
				eventAfterPrompt_ofe.nextAction();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		ImGui::Dummy({ paddingX, 0 }); ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0, 0, 1.0f));
		if (ImGui::Button("No", { buttonsizeX,0 }))
		{
			oo::EventManager::Broadcast(&eventAfterPrompt_ofe.nextEvent);
			if (eventAfterPrompt_ofe.nextAction && eventAfterPrompt_ofe.launchEventDesipteNo)
				eventAfterPrompt_ofe.nextAction();
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor(1);

		ImGui::SameLine();
		ImGui::Dummy({ paddingX, 0 }); ImGui::SameLine();
		if (ImGui::Button("Cancel", { buttonsizeX,0 }))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
}
