/************************************************************************************//*!
\file          ScriptingProperties.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Properties for displaying scripting variables on the editor. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "ScriptingProperties.h"
#include <string>
#include <Utility/UUID.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include "Quaternion/include/Quaternion.h"
#include "Ouroboros/ECS/GameObject.h"

#include "SceneManagement/include/SceneManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Scripting/ScriptComponent.h"
#include "Ouroboros/ECS/GameObject.h"

#include "App/Editor/Utility/ImGuiManager.h"
ScriptingProperties::ScriptingProperties()
{
	m_scriptUI.emplace(oo::ScriptValue::type_enum::BOOL, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			bool data = v.TryGetRuntimeValue().GetValue<bool>();
			bool edit = ImGui::Checkbox(v.name.c_str(), &data);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (edit) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); editing = edit; };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::INT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			int data = v.TryGetRuntimeValue().GetValue<int>();
			bool edit = ImGui::DragInt(v.name.c_str(), &data);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (edit) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); editing = edit;  };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::FLOAT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			float data = v.TryGetRuntimeValue().GetValue<float>();
			bool edit = ImGui::DragFloat(v.name.c_str(), &data);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (edit) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); editing = edit;  };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::STRING, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			std::string data = v.TryGetRuntimeValue().GetValue<std::string>();
			bool edit = ImGui::InputText(v.name.c_str(), &data);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (edit) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); editing = edit; };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::VECTOR2, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			oo::ScriptValue::vec2_type data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::vec2_type>();
			editing = ImGui::DragFloat2(v.name.c_str(), reinterpret_cast<float*>(&data),0.1f);
			edited = ImGui::IsItemDeactivatedAfterEdit();
            if (editing) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::VECTOR3, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
            oo::ScriptValue::vec3_type data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::vec3_type>();
			editing = ImGui::DragFloat3(v.name.c_str(), reinterpret_cast<float*>(&data), 0.1f);
			edited = ImGui::IsItemDeactivatedAfterEdit();
            if (editing) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::ENUM, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::enum_type>();
			auto list = data.GetOptions();

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::InputText(v.name.c_str(), &list[data.index]);
			ImGui::PopItemFlag();
			ImGui::SetItemAllowOverlap();
			
			static ImGuiID showList = 0;
			ImGuiID currID = ImGui::GetItemID();
			ImGui::SameLine(ImGui::CalcItemWidth() - 12.0f);
			if (ImGui::ArrowButton("Edit",ImGuiDir_::ImGuiDir_Down))
			{
				if (showList != currID)
					showList = currID;
				else
					showList = 0;
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
						v.TrySetRuntimeValue(oo::ScriptValue{ data });
						editing = true;
						edited = true;
						showList = 0;
						break;
					}
				}
				ImGui::EndListBox();
			}
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::LIST, [this](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::list_type>();
			int size = data.valueList.size();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::DragInt(v.name.c_str(), &size);
			ImGui::PopItemFlag();
			ImGui::SetItemAllowOverlap();

			float itemwidth = ImGui::CalcItemWidth();
			ImGui::SameLine(12.0f);
			if (ImGui::ArrowButton("##listenumleft", ImGuiDir_::ImGuiDir_Left))
			{
				if (--size < 0)
					size = 0;
				else
				{
					data.valueList.resize(size);
					editing = true; edited = true;
					v.TrySetRuntimeValue(oo::ScriptValue{ data });
					return;
				}
			}
			ImGui::SameLine(itemwidth - 12.0f);
			if (ImGui::ArrowButton("##listenumright", ImGuiDir_::ImGuiDir_Right))
			{
				data.Push();
				editing = true; edited = true;
				v.TrySetRuntimeValue(oo::ScriptValue{ data });
				return;
			}

			auto iter = m_scriptUI.find(data.type);
			int counter = 0;
			for (auto& item_value : data.valueList)
			{
				++counter;
				oo::ScriptFieldInfo val ("##tempname", item_value);
				bool current_editing = false;
				bool current_edited = false;
				ImGui::PushID(counter);
				iter->second(val, current_editing, current_edited);
				ImGui::PopID();
				if (current_editing)
				{
					editing = true;
					item_value = val.value;
					v.TrySetRuntimeValue(oo::ScriptValue{ data });
				}
				if (current_edited)
				{
					edited = true;
				}
			}
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::GAMEOBJECT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::UUID>();
			auto uuid = data.GetUUID();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			auto gameobject_ptr = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(data);
			std::string referenceObj = gameobject_ptr == nullptr ? "Invalid Object" : gameobject_ptr->Name();
			ImGui::InputText(v.name.c_str(), &referenceObj,ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemFlag();
			if (ImGui::BeginDragDropTarget())
			{
				auto* payload = ImGui::AcceptDragDropPayload("HIERARCHY_PAYLOAD");
				if (payload)
				{
					data = *static_cast<oo::UUID*>(payload->Data);
					editing = true;
					edited = true;
					if (editing) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); };
				}
				ImGui::EndDragDropTarget();
			}
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::FUNCTION, [this](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::function_type>();
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
					data.m_objID = *static_cast<oo::UUID*>(payload->Data);
					data.m_info.Reset();
					editing = true;
					edited = true;
				}
				ImGui::EndDragDropTarget();
			}
			if (editing) 
			{ 
				v.TrySetRuntimeValue(oo::ScriptValue{ data });
				return;
			};
			//function part
			ImGui::Separator();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			std::string funcName = data.m_info.functionName.size() ? data.m_info.functionName : "NOT ASSIGNED";
			ImGui::InputText("##Function", &funcName);
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
								ImGui::CloseCurrentPopup();
							}
						}
						ImGui::EndMenu();
					}
				}
				ImGui::EndPopup();
			}
			if (editing)
			{
				v.TrySetRuntimeValue(oo::ScriptValue{ data });
				return;
			};
			for (auto& param : data.m_info.paramList)
			{
				auto iter = m_scriptUI.find(param.value.GetValueType());
				if (m_scriptUI.end() == iter)
					continue;
				ImGui::PushID(ImGui::GetItemID());
				ImGui::PushItemWidth(50.0f);
				iter->second(param, editing, edited);
				ImGui::PopItemWidth();
				ImGui::PopID();
			}
			ImGui::Separator();
			if (editing)
			{
				v.TrySetRuntimeValue(oo::ScriptValue{ data });
				return;
			};
		});

}
