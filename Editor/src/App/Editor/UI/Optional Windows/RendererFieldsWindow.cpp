#include "pch.h"
#include "RendererFieldsWindow.h"
#include "Project.h"
#include "Ouroboros/Vulkan/GlobalRendererSettings.h"


RendererFieldsWindow::RendererFieldsWindow()
{
}
void RendererFieldsWindow::Show()
{
	rttr::type t = oo::RendererSettings::setting.get_type();
	for (auto prop : t.get_properties())
	{
		ImGui::Text(prop.get_name().data());
		if (ImGui::BeginChild(prop.get_name().data(), {0,150.0f},true))
		{
			rttr::variant v = prop.get_value(oo::RendererSettings::setting);
			bool edited = DisplaySetting(v);
			if(edited)
				prop.set_value(oo::RendererSettings::setting,v);
		}
		ImGui::EndChild();
	}
}
