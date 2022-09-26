#include "pch.h"
#include "InspectorProperties.h"
#include "UI_metadata.h"
#include <Utility/UUID.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <imgui/misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include "Quaternion/include/Quaternion.h"
#include "Ouroboros/ECS/GameObject.h"
#include "App/Editor/UI/Object Editor/AssetBrowser.h"
InspectorProperties::InspectorProperties()
{
	m_InspectorUI[UI_RTTRType::UItypes::BOOL_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		bool value = v.get_value<bool>();
		edited = ImGui::Checkbox(name.c_str(), &value);
		if (edited) { v = value; endEdit = true; };
	};
	m_InspectorUI[UI_RTTRType::UItypes::FLOAT_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		float value = v.get_value<float>();
		float speed = 1.0f;
		{
			rttr::variant variant_speed = prop.get_metadata(UI_metadata::DRAG_SPEED);
			if (variant_speed.is_valid())
				speed = variant_speed.get_value<float>();
		}
		edited = ImGui::DragFloat(name.c_str(), &value, speed);
		if (edited) { v = value; endEdit = true; };
	};
	m_InspectorUI[UI_RTTRType::UItypes::STRING_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.to_string();
		edited = ImGui::InputText(name.c_str(), &value);
		if (edited) { v = value; };
		endEdit = ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::VEC2_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::vec2>();
		float speed = 1.0f;
		{
			rttr::variant variant_speed = prop.get_metadata(UI_metadata::DRAG_SPEED);
			if (variant_speed.is_valid())
				speed = variant_speed.get_value<float>();
		}
		edited = ImGui::DragFloat2(name.c_str(), glm::value_ptr(value),speed);
		if (edited) { v = value; };
		endEdit = ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::VEC3_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::vec3>();
		float speed = 1.0f;
		{
			rttr::variant variant_speed = prop.get_metadata(UI_metadata::DRAG_SPEED);
			if (variant_speed.is_valid())
				speed = variant_speed.get_value<float>();
		}
		edited = ImGui::DragFloat3(name.c_str(), glm::value_ptr(value),speed);
		if (edited) { v = value; };
		endEdit = ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::VEC4_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::vec4>();
		float speed = 1.0f;
		{
			rttr::variant variant_speed = prop.get_metadata(UI_metadata::DRAG_SPEED);
			if (variant_speed.is_valid())
				speed = variant_speed.get_value<float>();
		}
		edited = ImGui::DragFloat4(name.c_str(), glm::value_ptr(value),speed);
		if (edited) { v = value; };
		endEdit = ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::UUID_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<UUID>().GetUUID();
		ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
		ImGui::InputScalarN(name.c_str(), ImGuiDataType_::ImGuiDataType_U64, &value, 1); //read only
		ImGui::PopItemFlag();
	};
	m_InspectorUI[UI_RTTRType::UItypes::MAT3_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::mat3>();
		edited |= ImGui::DragFloat3(name.c_str(), glm::value_ptr(value[0]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat3("##2", glm::value_ptr(value[1]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat3("##3", glm::value_ptr(value[2]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		if (edited) { v = value; };
	};
	m_InspectorUI[UI_RTTRType::UItypes::MAT4_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::mat4>();
		edited |= ImGui::DragFloat4(name.c_str(), glm::value_ptr(value[0]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat4("##2", glm::value_ptr(value[1]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat4("##3", glm::value_ptr(value[2]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat4("##4", glm::value_ptr(value[3]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		if (edited) { v = value; };
	};
	m_InspectorUI[UI_RTTRType::UItypes::QUAT_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<quaternion>().value;
		float speed = 1.0f;
		{
			rttr::variant variant_speed = prop.get_metadata(UI_metadata::DRAG_SPEED);
			if (variant_speed.is_valid())
				speed = variant_speed.get_value<float>();
		}
		edited = ImGui::DragFloat4(name.c_str(), glm::value_ptr(value),speed);
		if (edited) { v = value; };
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::DOUBLE_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<double>();
		float speed = 1.0f;
		{
			rttr::variant variant_speed = prop.get_metadata(UI_metadata::DRAG_SPEED);
			if (variant_speed.is_valid())
				speed = variant_speed.get_value<float>();
		}
		edited = ImGui::DragScalarN(name.c_str(), ImGuiDataType_::ImGuiDataType_Double, &value, 1,speed);
		if (edited) { v = value; };
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::SIZE_T_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<std::size_t>();
		float speed = 1.0f;
		{
			rttr::variant variant_speed = prop.get_metadata(UI_metadata::DRAG_SPEED);
			if (variant_speed.is_valid())
				speed = variant_speed.get_value<float>();
		}
		edited = ImGui::DragScalarN(name.c_str(), ImGuiDataType_::ImGuiDataType_U64, &value, 1,speed);
		if (edited) { v = value; };
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::ENTITY_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<Ecs::EntityID>();
		edited = ImGui::DragScalarN(name.c_str(), ImGuiDataType_::ImGuiDataType_U64, &value, 1);
		if (edited) { v = value; };
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::ASSET_TYPE] = [](rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<oo::Asset>();
		static ImGuiID open = 0;
		std::string string_temp = (value.IsValid())? value.GetFilePath().stem().string() : "Invalid Data";
		ImGui::InputText(name.c_str(), &string_temp,ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
		//if (value.IsValid() == false)
		//	return;
		ImGui::SameLine();
		ImGuiID temp = ImGui::GetItemID();
		if (ImGui::Button("Edit"))
		{
			if (open == temp)
				open = 0;
			else
			{
				open = temp;
			}
		}
		if (open == temp)
		{
			//this meta data is a required field
			rttr::variant asset_type = prop.get_metadata(UI_metadata::ASSET_TYPE);
			if(asset_type.is_valid())
				AssetBrowser::AssetPickerUI(v, endEdit, asset_type.get_value<int>());
		}
		if (endEdit)
		{
			edited = true;
			open = 0;
		}
	};
}
