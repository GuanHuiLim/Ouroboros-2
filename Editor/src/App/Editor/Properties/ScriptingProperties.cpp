#include "pch.h"
#include "ScriptingProperties.h"
#include <string>
#include <Utility/UUID.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
//#include <imgui/imgui/misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include "Quaternion/include/Quaternion.h"
#include "Ouroboros/ECS/GameObject.h"

#include "SceneManagement/include/SceneManager.h"
#include "Ouroboros/Scripting/ScriptComponent.h"
#include "Ouroboros/ECS/GameObject.h"

#include "App/Editor/Utility/ImGuiManager.h"
ScriptingProperties::ScriptingProperties()
{
	m_scriptUI.emplace(oo::ScriptValue::type_enum::BOOL, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			bool data = v.value.GetValue<bool>();
			bool edit = ImGui::Checkbox(v.name.c_str(), &data);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (edit) { v.value = oo::ScriptValue{ data }; editing = edit; };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::INT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			int data = v.value.GetValue<int>();
			bool edit = ImGui::DragInt(v.name.c_str(), &data);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (edit) { v.value = oo::ScriptValue{ data }; editing = edit;  };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::FLOAT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			float data = v.value.GetValue<float>();
			bool edit = ImGui::DragFloat(v.name.c_str(), &data);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (edit) { v.value = oo::ScriptValue{ data }; editing = edit;  };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::STRING, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			std::string data = v.value.GetValue<std::string>();
			bool edit = ImGui::InputText(v.name.c_str(), &data);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (edit) { v.value = oo::ScriptValue{ data }; editing = edit; };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::VECTOR2, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			glm::vec2 data = v.value.GetValue<glm::vec2>();
			editing = ImGui::DragFloat2(v.name.c_str(), glm::value_ptr(data),0.1f);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (editing) { v.value = oo::ScriptValue{ data }; };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::VECTOR3, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			glm::vec3 data = v.value.GetValue<glm::vec3>();
			editing = ImGui::DragFloat3(v.name.c_str(), glm::value_ptr(data), 0.1f);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (editing) { v.value = oo::ScriptValue{ data }; };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::ENUM, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.value.GetValue<oo::ScriptValue::enum_type>();
			auto list = data.GetOptions();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::InputText(data.name.c_str(), &list[data.index]);
			ImGui::PopItemFlag();
			static ImGuiID showList = 0;
			ImGuiID currID = ImGui::GetItemID();
			ImGui::SameLine();
			if (ImGui::Button("Edit"))
			{
				showList = currID;
				editing = true;
			}
			if (showList == currID)
			{
				ImGui::BeginListBox("##list");
				for (unsigned int i = 0 ; i < list.size(); ++i)
				{
					if (ImGui::Selectable(list[i].c_str()))
					{
						data.index = i;
						v.value = oo::ScriptValue{ data };
						editing = true;
						edited = true;
						showList = 0;
						break;
					}
				}
				ImGui::EndListBox();
			}
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::GAMEOBJECT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.value.GetValue<UUID>();
			auto uuid = data.GetUUID();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::DragScalarN(v.name.c_str(), ImGuiDataType_U64, &uuid, 1);
			ImGui::PopItemFlag();
			if (ImGui::BeginDragDropTarget())
			{
				auto* payload = ImGui::AcceptDragDropPayload("HIERARCHY_PAYLOAD");
				if (payload)
				{
					data = *static_cast<UUID*>(payload->Data);
					editing = true;
					edited = true;
					if (editing) { v.value = oo::ScriptValue{ data }; };
				}
				ImGui::EndDragDropTarget();
			}
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::FUNCTION, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.value.GetValue<oo::ScriptValue::function_type>();
			//assign UUID
			auto uuid = data.m_objID;
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::DragScalarN(v.name.c_str(), ImGuiDataType_U64, &uuid, 1);
			ImGui::PopItemFlag();
			if (ImGui::BeginDragDropTarget())
			{
				auto* payload = ImGui::AcceptDragDropPayload("HIERARCHY_PAYLOAD");
				if (payload)
				{
					data.m_objID = *static_cast<UUID*>(payload->Data);
					data.m_info.Reset();
					editing = true;
					edited = true;
				}
				ImGui::EndDragDropTarget();
			}
			if (editing) 
			{ 
				v.value = oo::ScriptValue{ data }; 
				return;
			};
			//function part
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::InputText("Function", &data.m_info.functionName);
			ImGui::PopItemFlag();

			auto gameobject = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(uuid);

			ImGui::SameLine();
			if (ImGui::SmallButton("o"))
				ImGui::OpenPopup("OpenOption");
			if (ImGui::BeginPopup("OpenOption"))
			{
				auto& scriptMap = gameobject->GetComponent<oo::ScriptComponent>().GetScriptInfoAll();
				for (auto& script : scriptMap)
				{
					if (ImGui::BeginMenu(script.first.c_str()))
					{
						for (auto fnc : script.second.classInfo.GetFunctionInfoAll())
						{
							if (ImGui::MenuItem(fnc.functionName.c_str()))
							{
								data.m_info = fnc;
								editing = true;
								edited = true;
								v.value = oo::ScriptValue{ data };
								ImGui::CloseCurrentPopup();
							}
						}
						ImGui::EndMenu();
					}
				}
				ImGui::EndPopup();
			}
		});

}
