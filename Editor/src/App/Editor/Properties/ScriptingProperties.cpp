#include "pch.h"
#include "ScriptingProperties.h"
#include <Utility/UUID.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui/misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include "Quaternion/include/Quaternion.h"
#include "Ouroboros/ECS/GameObject.h"
ScriptingProperties::ScriptingProperties()
{
	m_scriptUI.emplace(oo::ScriptValue::type_enum::BOOL, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			bool data = v.value.GetValue<bool>();
			bool edit = ImGui::Checkbox(v.name.c_str(), &data);
			if (edit) { v.value = oo::ScriptValue{ data }; editing = true; edited = true; };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::INT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			int data = v.value.GetValue<int>();
			bool edit = ImGui::DragInt(v.name.c_str(), &data);
			if (edit) { v.value = oo::ScriptValue{ data }; editing = true; edited = true; };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::FLOAT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			float data = v.value.GetValue<float>();
			bool edit = ImGui::DragFloat(v.name.c_str(), &data);
			if (edit) { v.value = oo::ScriptValue{ data }; editing = true; edited = true; };
		});

}
