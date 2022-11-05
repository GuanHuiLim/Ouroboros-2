/************************************************************************************//*!
\file           Serializer.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          uses rttr and rapidjson to help serializing
				Saves scenes/prefabs
				Loads scenes/prefabs
				Saving individual object
				Loading individual object 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Scene/Scene.h>
#include <rttr/variant.h>
#include <rttr/property.h>

#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>//rapidjson
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

#include <unordered_map>
#include <functional>
#include <string>

#include "App/Editor/Properties/UI_RTTRType.h"
#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Utility/ImGuiManager.h"

#include <Ouroboros/Prefab/PrefabSceneController.h>
#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/Transform/TransformSystem.h>

#include "Ouroboros/Prefab/PrefabComponent.h"
#include "Ouroboros/Scene/Scene.h"

#include "App/Editor/Properties/SerializerProperties.h"
#include "App/Editor/Properties/SerializerScriptingProperties.h"
#include "Project.h"
class Serializer
{
public:
	Serializer();
	~Serializer();
	static void InitEvents();
	/*********************************************************************************//*!
	\brief     This Function should be called before any usuage of serializer
	*//**********************************************************************************/
	static void Init();
	/*********************************************************************************//*!
	\brief      Save the current Scene
	 
	\param      scene ==> read through the scene file
	
	*//**********************************************************************************/
	static void SaveScene(oo::Scene& scene);
	/*********************************************************************************//*!
	\brief      Load the Scene base on the filepath suggested by scenefile
	 
	\param      scene ==> will populate this scene
	
	*//**********************************************************************************/
	static void LoadScene(oo::Scene& scene);

	static std::filesystem::path SavePrefab(std::shared_ptr<oo::GameObject> go, oo::Scene& scene);
	static oo::UUID LoadPrefab(std::filesystem::path path,std::shared_ptr<oo::GameObject> go,oo::Scene & scene);

	static std::string SaveDeletedObject(std::shared_ptr<oo::GameObject> go,oo::Scene& scene);
	static std::string SaveObjectsAsString(const std::vector<std::shared_ptr<oo::GameObject>>& go_list, oo::Scene& scene);

	static oo::UUID LoadDeleteObject(std::string& data, oo::UUID parentID, oo::Scene& scene);
	/*********************************************************************************//*!
	\brief      Recreation of the object 
	*//**********************************************************************************/
	static std::vector<oo::UUID> LoadObjectsFromString(std::string& data, oo::UUID parentID, oo::Scene& scene);

	//saving and loading single variant
	static std::string SaveSingleVariant(rttr::type t, rttr::property prop, rttr::variant v);
	template<typename Component>
	static void LoadSingleVariant(oo::GameObject& go, rttr::variant& var, rttr::property& prop, const std::string& data);
private:
	//saving
	static void Saving(std::stack<scenenode::raw_pointer>& s , std::stack<scenenode::handle_type>& parents,oo::Scene& scene, rapidjson::Document& doc);
	static void SaveObject(oo::GameObject& go, rapidjson::Value & val,rapidjson::Document& doc);
	static void SavePrefabObject(oo::GameObject& go, rapidjson::Value& val, rapidjson::Document& doc);
	static void SavePrefabObject_SubValues(rapidjson::Value& current, const rapidjson::Value& original);

	template <typename Component>
	static void SaveComponent(oo::GameObject& go, rapidjson::Value& val, rapidjson::Document& doc);
	static void SaveSequentialContainer(rttr::variant variant, rapidjson::Value& val, rttr::property prop,rapidjson::Document& doc);
	static void SaveNestedComponent(rttr::variant var, rapidjson::Value& val, rttr::property prop,rapidjson::Document& doc);
	//for saving 1 variant
	static void SaveVariant(rttr::type t,rttr::property prop, rttr::variant v, rapidjson::Value& val, rapidjson::Document& doc);
	//loading
	static oo::UUID Loading(std::shared_ptr<oo::GameObject> starting, oo::Scene& scene,rapidjson::Document & doc);
	static void LoadObject(oo::GameObject& go, rapidjson::Value::MemberIterator& iter, rapidjson::Value::MemberIterator& end);
	template <typename Component>
	static void LoadComponent(oo::GameObject& go, rapidjson::Value&& val);
	static void LoadSequentialContainer(rttr::variant& variant, rapidjson::Value& val);
	static void LoadNestedComponent(rttr::variant& variant, rapidjson::Value& val);
	//loading 1 variant
	template <typename Component>
	static void LoadVariant(oo::GameObject& go,rttr::variant& var, rttr::property& prop, rapidjson::Document& doc);
	//creation
	static oo::UUID CreatePrefab(std::shared_ptr<oo::GameObject> starting, oo::Scene& scene, std::filesystem::path& p);
	//scripts
	static void SaveScript(oo::GameObject& go,rapidjson::Value& val,rapidjson::Document& doc);
	static void LoadScript(oo::GameObject& go,rapidjson::Value&& val);
	static void RemapScripts(std::unordered_map<oo::UUID, oo::UUID>& scriptIds, oo::GameObject& go);
protected://serialzation helpers
	template <typename Component>
	static void AddLoadComponent() noexcept;
private:
	//the function requires the user to insert the variant into the value manually
	//saving
	inline static SerializerSaveProperties m_SaveProperties;
	inline static SerializerScriptingSaveProperties m_saveScriptProperties;
	//loading
	inline static std::unordered_map < rttr::type::type_id, std::function<void(oo::GameObject&, rapidjson::Value&&)>> load_components;
	inline static SerializerLoadProperties m_LoadProperties;
	inline static SerializerScriptingLoadProperties m_loadScriptProperties;
	inline static constexpr int rapidjson_precision = 4;
	inline static constexpr float rapidjson_epsilon = 0.0001f;
};

template<typename Component>
inline void Serializer::LoadSingleVariant(oo::GameObject& go, rttr::variant& var, rttr::property& prop, const std::string& data)
{
	rapidjson::StringStream stream(data.c_str());
	rapidjson::Document doc;
	doc.ParseStream(stream);
	LoadVariant<Component>(go,var,prop, doc);
}

template<typename Component>
inline void Serializer::SaveComponent(oo::GameObject& go, rapidjson::Value& val,rapidjson::Document& doc)
{
	if (go.HasComponent<Component>() == false)
		return;
	rapidjson::Value v(rapidjson::kObjectType);
	v.SetObject();
	Component& component = go.GetComponent<Component>();
	rttr::type type = component.get_type();
	for (auto& prop : type.get_properties())
	{
		if (prop.is_readonly())
			continue;
		auto prop_type = prop.get_type();
		auto iter = UI_RTTRType::types.find(prop_type.get_id());
		if (iter == UI_RTTRType::types.end())
		{
			if (prop_type.is_sequential_container())
			{
				rttr::variant variant = prop.get_value(component);
				SaveSequentialContainer(variant, v, prop,doc);
			}
			else if (prop_type.is_class())
			{
				rttr::variant component_variant = prop.get_value(component);
				SaveNestedComponent(component_variant, v, prop,doc);
			}
			else if (prop_type.is_enumeration())
			{
				rttr::variant enum_data = prop.get_value(component);
				int value = enum_data.get_value<int>();
				//saves all enum data as int
				auto rttrType = UI_RTTRType::types.find(rttr::type::get<int>().get_id());
				m_SaveProperties.m_save_commands.find(rttrType->second)->second(doc,v,value,prop);
			}
			continue;//not supported
		}
		auto sf = m_SaveProperties.m_save_commands.find(iter->second);
		if (sf == m_SaveProperties.m_save_commands.end())
			continue;//don't have this save function
		sf->second(doc,v,prop.get_value(component),prop);
	}
	std::string temp = type.get_name().data();
	rapidjson::Value name;
	name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()),doc.GetAllocator());
	val.AddMember(name , v, doc.GetAllocator());
}

template<typename Component>
inline void Serializer::LoadComponent(oo::GameObject& go, rapidjson::Value&& val)
{
	if (go.HasComponent<Component>() == false)
	{
		std::string msg = "GameObject does not contain this component: ";
		msg += rttr::type::get<Component>().get_name().data();
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, msg);
	}
	Component& component = go.GetComponent<Component>();
	for (auto iter = val.MemberBegin(); iter != val.MemberEnd(); ++iter)
	{
		rttr::type t = component.get_type();
		rttr::property prop = t.get_property(iter->name.GetString());
		if (prop.is_valid() == false)
			continue;
		
		auto types_UI = UI_RTTRType::types.find(prop.get_type().get_id());
		if (types_UI == UI_RTTRType::types.end())
		{
			rttr::type prop_type = prop.get_type();
			if (prop_type.is_sequential_container())
			{
				rttr::variant v = prop.get_value(component);
				LoadSequentialContainer(v, iter->value);
				prop.set_value(component, v);
			}
			if (prop_type.is_class())
			{
				rttr::variant variant = prop.get_value(component);
				LoadNestedComponent(variant, iter->value);
				prop.set_value(component, variant);
			}
			else if (prop_type.is_enumeration())
			{
				rttr::variant enum_data = prop.get_value(component);
				
				//saves all enum data as int
				auto rttrType = UI_RTTRType::types.find(rttr::type::get<int>().get_id());
				m_LoadProperties.m_load_commands.find(rttrType->second)->second(enum_data, std::move(iter->value));
				prop.set_value(component, enum_data);
			}
			continue;//not supported
		}
		auto command = m_LoadProperties.m_load_commands.find(types_UI->second);
		if (command == m_LoadProperties.m_load_commands.end())
			continue;//don't have this save function
		rttr::variant v;
		command->second(v, std::move(iter->value));
		prop.set_value(component, v);
	}
}
template<typename Component>
inline void Serializer::LoadVariant(oo::GameObject& go, rttr::variant& var, rttr::property& prop, rapidjson::Document& doc)
{
	if (go.HasComponent<Component>() == false)
		return;
	Component& component = go.GetComponent<Component>();
	auto member = doc.GetObj().MemberBegin();
	rttr::type t = rttr::type::get_by_name(member->name.GetString());
	auto propertyData = member->value.MemberBegin();
	prop = t.get_property(propertyData->name.GetString());
	if (prop.is_valid() == false)
		return;

	auto types_UI = UI_RTTRType::types.find(prop.get_type().get_id());
	if (types_UI == UI_RTTRType::types.end())
	{
		rttr::type prop_type = prop.get_type();
		if (prop_type.is_sequential_container())
		{
			rttr::variant v = prop.get_value(component);
			LoadSequentialContainer(v, propertyData->value);
			prop.set_value(var, v);
		}
		if (prop_type.is_class())
		{
			rttr::variant variant = prop.get_value(component);
			LoadNestedComponent(variant, propertyData->value);
			prop.set_value(var, variant);
		}
		else if (prop_type.is_enumeration())
		{
			rttr::variant enum_data = prop.get_value(component);

			//saves all enum data as int
			auto rttrType = UI_RTTRType::types.find(rttr::type::get<int>().get_id());
			m_LoadProperties.m_load_commands.find(rttrType->second)->second(enum_data, std::move(propertyData->value));
			prop.set_value(var, enum_data);
		}
		return;//not supported
	}
	auto command = m_LoadProperties.m_load_commands.find(types_UI->second);
	if (command == m_LoadProperties.m_load_commands.end())
		return;//don't have this save function
	
	command->second(var, std::move(propertyData->value));

}
/*********************************************************************************//*!
\brief      This Function is slow as it dosent do data copying for the prefab.
			The speed is just using rapidjson
*//**********************************************************************************/
template<>
inline void Serializer::LoadComponent<oo::PrefabComponent>(oo::GameObject& go, rapidjson::Value&& val)
{
	if (go.HasComponent<oo::PrefabComponent>() == false)
	{
		std::string msg = "GameObject does not contain this component: ";
		msg += rttr::type::get<oo::PrefabComponent>().get_name().data();
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, msg);
	}
	oo::PrefabComponent& component = go.GetComponent<oo::PrefabComponent>();
	component.prefab_filePath = val.FindMember("File Path")->value.GetString();

	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();

	std::string& data = ImGuiManager::s_prefab_controller->RequestForPrefab((Project::GetPrefabFolder()/component.prefab_filePath).string());
	rapidjson::StringStream stream(data.c_str());
	
	rapidjson::Document document;
	document.ParseStream(stream);
	std::unordered_map<oo::UUID, oo::UUID> scripts_id_mapping;
	std::vector<std::shared_ptr<oo::GameObject>> all_objects;
	std::stack<std::shared_ptr<oo::GameObject>> parents;
	std::shared_ptr<oo::GameObject> gameobj = scene->FindWithInstanceID(go.GetInstanceID());
	std::vector<std::shared_ptr<oo::GameObject>> second_iter;
	parents.push(gameobj);
	for (auto iter = document.MemberBegin(); iter != document.MemberEnd();)
	{
		//map their old id to their current IDs
		scripts_id_mapping.emplace(std::stoull(iter->name.GetString()), gameobj->GetInstanceID());
		all_objects.push_back(gameobj);
		gameobj->SetIsPrefab(true);

		auto members = iter->value.MemberBegin();//get the order of hierarchy
		int order = members->value.GetInt();
		{
			//when the order dont match the size it will keep poping until it matches
			//then parent to it and adds itself
			while (order != parents.size())
				parents.pop();

			parents.top()->AddChild(*gameobj, true);
			parents.push(gameobj);
		}

		second_iter.emplace_back(gameobj);
		++iter;
		if (iter != document.MemberEnd())
		{
			gameobj = scene->CreateGameObjectImmediate();
		}

	}
	
	scene->GetWorld().Get_System<oo::TransformSystem>()->UpdateSubTree(go, false);

	int iteration = 0;
	for (auto iter = document.MemberBegin(); iter != document.MemberEnd(); ++iter, ++iteration)
	{
		auto child_object = second_iter[iteration];
		auto members = iter->value.MemberBegin(); //get the order of hierarchy
		auto membersEnd = iter->value.MemberEnd();

		++members;

		//processes the components		
		LoadObject(*child_object, members, membersEnd);
		if (val.HasMember(iter->name))
		{
			auto& overide_component = val.FindMember(iter->name)->value;
			auto overideBegin = overide_component.MemberBegin();
			auto overideEnd = overide_component.MemberEnd();
			LoadObject(*child_object, overideBegin, overideEnd);
		}
	}

	//remapping of scripts here.
	for (auto go_ptr : all_objects)
	{
		RemapScripts(scripts_id_mapping, *go_ptr);
	}
}

template<typename Component>
inline void Serializer::AddLoadComponent() noexcept
{
	load_components.emplace(rttr::type::get<Component>().get_id(),
		[](oo::GameObject& go, rapidjson::Value&& v) 
		{
			go.EnsureComponent<Component>();
			LoadComponent<Component>(go, std::move(v));
		});
}

template <>
inline void Serializer::AddLoadComponent<oo::PrefabComponent>() noexcept
{
	load_components.emplace(rttr::type::get<oo::PrefabComponent>().get_id(),
		[](oo::GameObject& go, rapidjson::Value&& v)
		{
			go.EnsureComponent<oo::PrefabComponent>();
			LoadComponent<oo::PrefabComponent>(go, std::move(v));
		});
}

