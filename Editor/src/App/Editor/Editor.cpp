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
Editor::Editor()
{
	UI_RTTRType::Init();
	Serializer::Init();//runs the init function
	Serializer::InitEvents();
	oo::CommandStackManager::InitEvents();
	ImGuiManager::Create("Hierarchy", true, ImGuiWindowFlags_MenuBar, [this] {this->m_hierarchy.Show(); });
	ImGuiManager::Create("Inspector", true, ImGuiWindowFlags_MenuBar, [this] {this->m_inspector.Show(); });
	ImGuiManager::Create("FileBrowser", true, ImGuiWindowFlags_MenuBar, [this] {this->m_fileBrowser.Show(); });
	ImGuiManager::Create("Script Sequencer", true, ImGuiWindowFlags_None, [this] {this->m_scriptSequencer.Show(); });

	ImGuiManager::Create("Style Editor", true, ImGuiWindowFlags_MenuBar, [this] {this->m_styleEditor.Show(); });
	ImGuiManager::Create("PenTool", false, (ImGuiWindowFlags_)(ImGuiWindowFlags_NoDecoration), [this] {this->m_pentool.Show(); });
	ImGuiManager::Create("Toolbar", true, ImGuiWindowFlags_None, [this] {this->m_toolbar.Show(); });
	ImGuiManager::Create("Logger", true, (ImGuiWindowFlags_)(ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar), [this] {this->m_loggingView.Show(); });

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
		m_styleEditor.InitStyle(); 
		return true; 
	}();
	ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	MenuBar();

	ImGuiManager::UpdateAllUI();
	m_warningMessage.Show();
	helper.Popups();
	if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_S))
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		Serializer::SaveScene(*(scene));
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Scene Saved");
	}
	if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_Z))
	{
		oo::CommandStackManager::UndoCommand();
	}
	if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_Y))
	{
		oo::CommandStackManager::RedoCommand();
	}

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
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
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
	if (e->nextAction.nextEvent.m_type != OpenFileEvent::FileType::SCENE)
		return;
	if (ImGui::IsPopupOpen(ImGuiID{ 0 }, ImGuiPopupFlags_::ImGuiPopupFlags_AnyPopup))
		return;

	eventAfterPrompt_ofe = e->nextAction;
	openfile = true;
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
			oo::EventManager::Broadcast(&eventAfterPrompt.nextEvent);
			if (eventAfterPrompt.nextAction)
				eventAfterPrompt.nextAction();
			ImGui::CloseCurrentPopup();
			//save the scene
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			Serializer::SaveScene(*(scene));
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Scene Saved");
		}
		ImGui::SameLine();
		ImGui::Dummy({ paddingX, 0 }); ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0, 0, 1.0f));
		if (ImGui::Button("No", { buttonsizeX,0 }))
		{
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
