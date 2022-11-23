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

#include "App/Editor/UI/Object Editor/AssetBrowser.h"
#include <Project.h>

//this is a little of a fuckfest
#include "App/Editor/UI/Object Editor/Hierarchy.h"
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
	m_scriptUI.emplace(oo::ScriptValue::type_enum::COLOR, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			oo::Color data = v.TryGetRuntimeValue().GetValue<oo::Color>();
			editing = ImGui::ColorEdit4(v.name.c_str(), &data.r, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_InputRGB);
			edited = ImGui::IsItemDeactivatedAfterEdit();
			if (editing) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); };
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::ENUM, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::enum_type>();
			auto list = data.GetOptions();
            auto name = data.GetValueName(data.value);

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImGui::InputText(v.name.c_str(), &name);
			ImGui::PopItemFlag();
			ImGui::SetItemAllowOverlap();
			
			static ImGuiID showList = 0;
			ImGuiID currID = ImGui::GetItemID();
			ImGui::SetCursorPos(cursorPos);
			ImGui::Dummy({ ImGui::CalcItemWidth() - ImGui::GetStyle().ItemSpacing.x * 2 - 12.0f  ,0 }); ImGui::SameLine();
			if (ImGui::ArrowButton("Edit",ImGuiDir_::ImGuiDir_Down))
			{
				if (showList != currID)
					showList = currID;
				else
					showList = 0;
			}
			if (showList == currID)
			{
				ImGui::BeginListBox("##list");
				for (unsigned int i = 0 ; i < list.size(); ++i)
				{
					if (ImGui::Selectable(list[i].c_str()))
					{
                        data.value = data.GetValues()[i];
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
			int size = static_cast<int>(data.valueList.size());
			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::DragInt(v.name.c_str(), &size);
			ImGui::PopItemFlag();
			ImGui::SetItemAllowOverlap();
			ImGui::PushID(v.name.c_str());
			float itemwidth = ImGui::CalcItemWidth();
			ImGui::SetCursorPos(cursorPos);
			if (ImGui::ArrowButton("##listenumleft", ImGuiDir_::ImGuiDir_Left))
			{
				if (--size < 0)
					size = 0;
				else
				{
					data.valueList.resize(size);
					editing = true; edited = true;
					v.TrySetRuntimeValue(oo::ScriptValue{ data });
					ImGui::PopID();
					return;
				}
			}


			ImGui::SetCursorPos(cursorPos);
			ImGui::Dummy({ itemwidth - ImGui::GetStyle().ItemSpacing.x * 2 - 12.0f ,0 }); ImGui::SameLine();
			if (ImGui::ArrowButton("##listenumright", ImGuiDir_::ImGuiDir_Right))
			{
				data.Push();
				editing = true; edited = true;
				v.TrySetRuntimeValue(oo::ScriptValue{ data });
				ImGui::PopID();
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
			if (data.valueList.size())
			{
				ImGui::SameLine(0,5.0f);
				ImGui::TextColored(ImVec4(1.0f,0,0,1.0f), "End of list");
			}
			ImGui::PopID();
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::GAMEOBJECT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::UUID>();
			//ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			auto gameobject_ptr = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(data);
			std::string referenceObj = gameobject_ptr == nullptr ? "Invalid Object" : gameobject_ptr->Name();
			ImGui::InputText(v.name.c_str(), &referenceObj,ImGuiInputTextFlags_ReadOnly);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && gameobject_ptr)
				Hierarchy::SetItemSelected(data);
			//ImGui::PopItemFlag();
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
	m_scriptUI.emplace(oo::ScriptValue::type_enum::PREFAB, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::prefab_type>();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			std::string referenceObj = data.filePath.empty()? "Invalid Object" : std::filesystem::path(data.filePath).filename().string();
			ImGui::InputText(v.name.c_str(), &referenceObj, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemFlag();
			if (ImGui::BeginDragDropTarget())
			{
				auto* payload = ImGui::AcceptDragDropPayload(".prefab");
				if (payload)
				{
					auto path = (*static_cast<std::filesystem::path*>(payload->Data));
					data.filePath = std::filesystem::relative(path, Project::GetPrefabFolder()).string();
					editing = true;
					edited = true;
					if (editing) { v.TrySetRuntimeValue(oo::ScriptValue{ data }); };
				}
				ImGui::EndDragDropTarget();
			}
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::COMPONENT, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::component_type>();
			//ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			auto gameobject_ptr = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(data.m_objID);
			std::string referenceObj = gameobject_ptr == nullptr ? "Invalid "+ data.m_name : gameobject_ptr->Name();
			std::string intput_txt_name = data.m_name + " " + v.name;
			ImGui::InputText(intput_txt_name.c_str(), &referenceObj, ImGuiInputTextFlags_ReadOnly);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && gameobject_ptr)
				Hierarchy::SetItemSelected(data.m_objID);
			//ImGui::PopItemFlag();
			if (ImGui::BeginDragDropTarget())
			{
				auto* payload = ImGui::AcceptDragDropPayload("HIERARCHY_PAYLOAD");
				if (payload)
				{
					data.m_objID = *static_cast<oo::UUID*>(payload->Data);
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
			

			auto gameobject = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(uuid);
			bool validGameObject = gameobject == nullptr;
			std::string name = validGameObject ? "Invalid" : gameobject->Name();
			//ImGui::PushItemFlag(ImGuiItemFlags_Disabled,true);
			ImGui::InputText(v.name.c_str(), &name,ImGuiInputTextFlags_ReadOnly);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !validGameObject)
				Hierarchy::SetItemSelected(uuid);
			//ImGui::PopItemFlag();


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
			if (validGameObject)
				return;//nothing to display so dont bother


			
			//function part
			std::string funcName = data.m_info.functionName.size() ? data.m_info.functionName : "NOT ASSIGNED";
			ImGui::Separator();
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::InputText("##Function", &funcName);
			ImGui::PopItemFlag();


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
	m_scriptUI.emplace(oo::ScriptValue::type_enum::CLASS, [this](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			auto data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::class_type>();
			std::string name = "Class Type" + data.name_space + data.name +" "+ v.name;
			
			if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen))
			{
				ImGui::PushID(v.name.c_str());
				ImGui::Dummy({5,0}); ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::Separator();
				for (auto& sfi : data.infoList)
				{
					auto iter = m_scriptUI.find(sfi.value.GetValueType());
					if (iter != m_scriptUI.end())
					{
						bool editing_sub = false;
						bool edited_sub = false;
						iter->second(sfi, editing_sub, edited_sub);
						editing |= editing_sub;
						edited |= edited_sub;
					}
				}
				ImGui::Separator();
				ImGui::Separator();
				ImGui::EndGroup();
				ImGui::PopID();
			}
			if(editing)
				v.TrySetRuntimeValue(oo::ScriptValue{ data });
			
		});
	m_scriptUI.emplace(oo::ScriptValue::type_enum::ASSET, [](oo::ScriptFieldInfo& v, bool& editing, bool& edited)
		{
			oo::ScriptValue::asset_type data = v.TryGetRuntimeValue().GetValue<oo::ScriptValue::asset_type>();
			static ImGuiID opened = 0;
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			std::string temp = data.asset.GetFilePath().stem().string();
			ImGui::InputText(v.name.c_str(),&temp);
			ImGui::PopItemFlag();
			ImGui::SameLine();

			ImGuiID curr = ImGui::GetItemID();
			ImGui::PushID(v.name.c_str());
			if (ImGui::Button("edit"))
			{
				if (curr == opened)
					opened = 0;
				else
					opened = curr;
			}
			ImGui::PopID();
			if (curr == opened)
			{
				rttr::variant asset_data = data;
				AssetBrowser::AssetPickerUI(asset_data ,edited, (int)data.type);
				data.asset = asset_data.get_value<oo::Asset>();
			}
			if (edited)
			{
				editing = true;
				opened = 0;
				v.TrySetRuntimeValue(oo::ScriptValue{ data });
			}
		});
}
