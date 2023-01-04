#include "pch.h"
#include "RendererFieldsWindow.h"
#include "Project.h"
#include "Ouroboros/Vulkan/GlobalRendererSettings.h"

#include "Ouroboros/EventSystem/EventTypes.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include <App/Editor/UI/Tools/WarningMessage.h>

RendererFieldsWindow::RendererFieldsWindow()
{
}

void RendererFieldsWindow::Show()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Save"))
		{
			Project::SaveRendererSettingFile();
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Saved(Renderer Settings)");
		}
		if (ImGui::MenuItem("Load/Revert"))
		{
			Project::LoadRendererSettingFile();
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Loaded(Renderer Settings)");
		}
		ImGui::EndMenuBar();
	}
	bool isEdited = false;

	rttr::type t = oo::RendererSettings::setting.get_type();
	for (auto prop : t.get_properties())
	{
		ImGui::Text(prop.get_name().data());
		if (ImGui::BeginChild(prop.get_name().data(), {0,150.0f},true))
		{
			rttr::variant v = prop.get_value(oo::RendererSettings::setting);
			bool edited = DisplaySetting(v);
			isEdited |= edited;
			if(edited)
				prop.set_value(oo::RendererSettings::setting,v);
		}
		ImGui::EndChild();
	}

	if (isEdited)
	{
		UpdateRendererSettings e;
		oo::EventManager::Broadcast<UpdateRendererSettings>(&e);
	}
}
