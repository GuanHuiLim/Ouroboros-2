/************************************************************************************//*!
\file           Serializer.cpp
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          Saves scenes/prefabs
				Loads scenes/prefabs
				Saving individual object
				Loading individual object

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Serializer.h"

#include <Scenegraph/include/scenenode.h>
#include <Scenegraph/include/Scenegraph.h>
#include <SceneManagement/include/SceneManager.h>

#include <Ouroboros/ECS/GameObject.h>

#include <Ouroboros/EventSystem/EventManager.h>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "Project.h"

#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Events/LoadSceneEvent.h"
#include "Ouroboros/Scripting/ScriptComponent.h"
#include "Ouroboros/Scripting/ScriptManager.h"
#include <Ouroboros/Vulkan/RendererComponent.h>
#include <Ouroboros/Physics/ColliderComponents.h>
#include <Ouroboros/Physics/RigidbodyComponent.h>
#include <Ouroboros/Vulkan/LightComponent.h>
#include "Ouroboros/Audio/AudioListenerComponent.h"
#include "Ouroboros/Audio/AudioSourceComponent.h"

#include <Ouroboros/Transform/TransformSystem.h>

Serializer::Serializer()
{
}

Serializer::~Serializer()
{
}

void Serializer::InitEvents()
{
	oo::EventManager::Subscribe<LoadSceneEvent>([](LoadSceneEvent* loadscene) {
		Serializer::LoadScene(*loadscene->m_scene);
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Scene Loaded");
		});
}

void Serializer::Init()
{
	AddLoadComponent<oo::GameObjectComponent>();
	AddLoadComponent<oo::TransformComponent>();
	AddLoadComponent<oo::PrefabComponent>();
	AddLoadComponent<oo::MeshRendererComponent>();
	AddLoadComponent<oo::LightComponent>();
	AddLoadComponent<oo::RigidbodyComponent>();
	AddLoadComponent<oo::CapsuleColliderComponent>();
	AddLoadComponent<oo::BoxColliderComponent>();
	AddLoadComponent<oo::SphereColliderComponent>();
	AddLoadComponent<oo::AudioListenerComponent>();
	AddLoadComponent<oo::AudioSourceComponent>();

	load_components.emplace(rttr::type::get<oo::ScriptComponent>().get_id(),
		[](oo::GameObject& go, rapidjson::Value&& v)
		{
			go.AddComponent<oo::ScriptComponent>();
			LoadScript(go, std::move(v));
		});
}

void Serializer::SaveScene(oo::Scene& scene)
{
	scenegraph sg = scene.GetGraph();

	std::stack<scenenode::raw_pointer> s;
	std::stack<scenenode::handle_type> parents;
	scenenode::raw_pointer curr = sg.get_root().get();

	parents.push(curr->get_handle());
	for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
	{
		scenenode::shared_pointer child = *iter;
		s.push(child.get());
	}
	rapidjson::Document doc;
	doc.SetObject();
	Saving(s,parents,scene,doc);
	std::ofstream ofs(scene.GetFilePath(), std::fstream::out | std::fstream::trunc);
	if (ofs.good())
	{
		rapidjson::OStreamWrapper osw(ofs);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatDefault);
		writer.SetMaxDecimalPlaces(rapidjson_precision);
		doc.Accept(writer);
		ResetDocument();
		ofs.close();
	}
}

void Serializer::LoadScene(oo::Scene& scene)
{
	std::ifstream ifs(scene.GetFilePath());
	if (ifs.peek() == std::ifstream::traits_type::eof())
	{
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "Scene File is not valid!");
		return;
	}
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);
	Loading(scene.GetRoot(),scene,doc);
	ifs.close();
}

std::filesystem::path Serializer::SavePrefab(std::shared_ptr<oo::GameObject> go , oo::Scene & scene)
{
	std::stack<scenenode::raw_pointer> s;
	std::stack<scenenode::handle_type> parents;
	scenenode::raw_pointer curr = (*go).GetSceneNode().lock().get();
	parents.push(curr->get_handle());
	s.push(curr);
	rapidjson::Document doc;
	doc.SetObject();
	Saving(s,parents, scene,doc);
	std::filesystem::path newprefabPath = go->Name() + ".prefab";
	std::ofstream ofs(Project::GetPrefabFolder().string() + go->Name() + ".prefab", std::fstream::out | std::fstream::trunc);
	if (ofs.good())
	{
		rapidjson::OStreamWrapper osw(ofs);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatDefault);
		writer.SetMaxDecimalPlaces(rapidjson_precision);
		doc.Accept(writer);
		ResetDocument();
		ofs.close();
	}
	return newprefabPath;
}

UUID Serializer::LoadPrefab(std::filesystem::path path, std::shared_ptr<oo::GameObject> parent , oo::Scene& scene)
{
	auto prefabID = CreatePrefab(parent, scene, path);
	return prefabID;
}

std::string Serializer::SaveDeletedObject(std::shared_ptr<oo::GameObject> go, oo::Scene& scene)
{
	std::stack<scenenode::raw_pointer> s;
	std::stack<scenenode::handle_type> parents;
	scenenode::raw_pointer curr = (*go).GetSceneNode().lock().get();
	parents.push(curr->get_handle());
	s.push(curr);
	rapidjson::Document doc;
	doc.SetObject();
	Saving(s,parents, scene, doc);
	size_t count = go->GetChildCount();
	rapidjson::StringBuffer buffer(0, count * 200);
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	std::string temp = buffer.GetString();
	return temp;
}

std::string Serializer::SaveObjectsAsString(const std::vector<std::shared_ptr<oo::GameObject>>& go_list, oo::Scene& scene)
{
	std::stack<scenenode::raw_pointer> s;
	std::stack<scenenode::handle_type> parents;
	for (auto& go : go_list)
	{
		scenenode::raw_pointer curr = (*go).GetSceneNode().lock().get();
		s.push(curr);
	}
	//parents.push((*go_list.begin())->GetSceneNode().lock()->get_handle());
	rapidjson::Document doc;
	doc.SetObject();
	Saving(s, parents, scene, doc);
	size_t count = 10;
	rapidjson::StringBuffer buffer(0, count * 200);
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	std::string temp = buffer.GetString();
	ImGui::SetClipboardText(temp.c_str());
	return temp;
}

UUID Serializer::LoadDeleteObject(std::string& data, UUID parentID, oo::Scene& scene)
{
	rapidjson::StringStream stream(data.c_str());
	rapidjson::Document doc;
	doc.ParseStream(stream);
	auto parent = scene.FindWithInstanceID(parentID);

	ASSERT_MSG(parent == nullptr, "parent not found");
	
	auto firstObj = Loading(parent,scene,doc);
	return firstObj;
}

std::vector<UUID> Serializer::LoadObjectsFromString(std::string& data, UUID parentID, oo::Scene& scene)
{
	rapidjson::StringStream stream(data.c_str());
	rapidjson::Document doc;
	doc.ParseStream(stream);
	auto starting = scene.FindWithInstanceID(parentID);

	ASSERT_MSG(starting == nullptr, "parent not found");

	UUID firstobj;
	std::stack<std::shared_ptr<oo::GameObject>> parents;
	std::vector<std::shared_ptr<oo::GameObject>> second_iter;
	std::vector<UUID> go_UUID;
	parents.push(starting);
	for (auto iter = doc.MemberBegin(); iter != doc.MemberEnd(); ++iter)
	{
		auto go = scene.CreateGameObjectImmediate();
		go_UUID.push_back(go->GetInstanceID());

		auto members = iter->value.MemberBegin();//get the order of hierarchy
		auto membersEnd = iter->value.MemberEnd();
		int order = members->value.GetInt();


		{//when the order dont match the size it will keep poping until it matches
		//then parent to it and adds itself
			while (order != parents.size())
				parents.pop();

			if (parents.size())
				parents.top()->AddChild(*go, true);
			else
				starting->AddChild(*go, true);

			parents.push(go);
			if (iter == doc.MemberBegin())
				firstobj = go->GetInstanceID();
		}

		second_iter.emplace_back(go);
	}

	scene.GetWorld().Get_System<oo::TransformSystem>()->UpdateSubTree(*starting, false);

	int iteration = 0;
	for (auto iter = doc.MemberBegin(); iter != doc.MemberEnd(); ++iter, ++iteration)
	{
		auto go = second_iter[iteration];
		auto members = iter->value.MemberBegin();//get the order of hierarchy
		auto membersEnd = iter->value.MemberEnd();
		int order = members->value.GetInt();

		++members;
		//processes the components		
		LoadObject(*go, members, membersEnd);
	}

	return go_UUID;
}

void Serializer::Saving(std::stack<scenenode::raw_pointer>& s, std::stack<scenenode::handle_type>& parents, oo::Scene& scene, rapidjson::Document& doc)
{
	scenenode::raw_pointer curr;
	while (!s.empty())
	{
		curr = s.top();
		s.pop();
		auto go = scene.FindWithInstanceID(curr->get_handle());
		//bulk of code here
		rapidjson::Value name;
		std::string id = std::to_string(curr->get_handle());
		name.SetString(id.c_str(),static_cast<rapidjson::SizeType>(id.size()),doc.GetAllocator());

		rapidjson::Value gameobject_start;
		gameobject_start.SetObject();
		gameobject_start.AddMember("Order", parents.size(), doc.GetAllocator());

		bool is_prefab = go->HasComponent<oo::PrefabComponent>();
		is_prefab ? SavePrefabObject(*go, gameobject_start,doc) : SaveObject(*go, gameobject_start,doc);


		doc.AddMember(name, gameobject_start, doc.GetAllocator());
		//end of writing this gameobject
		if (curr->get_direct_child_count() && is_prefab == false)//if there is childs
		{
			parents.push(curr->get_handle());
			for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
			{
				scenenode::shared_pointer child = *iter;
				s.push(child.get());
			}
		}
		else if (s.empty() == false)
		{
			auto parent_handle = s.top()->get_parent_handle();
			while (parents.empty() == false)
			{
				auto c_handle = parents.top();
				if (c_handle == parent_handle)
				{
					break;
				}
				else
				{
					parents.pop();
				}
			}
		}
	}
}

void Serializer::SaveObject(oo::GameObject& go, rapidjson::Value& val,rapidjson::Document& doc)
{
	//will have more components
	SaveComponent<oo::GameObjectComponent>(go, val,doc);
	SaveComponent<oo::TransformComponent>(go, val,doc);

	SaveComponent<oo::MeshRendererComponent>(go, val, doc);
	SaveComponent<oo::LightComponent>(go, val, doc);

	SaveComponent<oo::AudioListenerComponent>(go, val, doc);
	SaveComponent<oo::AudioSourceComponent>(go, val, doc);

	SaveComponent<oo::RigidbodyComponent>(go, val, doc);
	SaveComponent<oo::BoxColliderComponent>(go, val, doc);
	SaveComponent<oo::CapsuleColliderComponent>(go, val, doc);
	SaveComponent<oo::SphereColliderComponent>(go, val, doc);

	SaveScript(go, val, doc);// this is the last item
}

void Serializer::SavePrefabObject(oo::GameObject& go, rapidjson::Value& val,rapidjson::Document& doc)
{
	//save everything
	SaveComponent<oo::PrefabComponent>(go, val, doc);
	SaveScript(go, val, doc);

	auto rpj_prefabComponent = val.FindMember(rttr::type::get<oo::PrefabComponent>().get_name().data());

	auto& prefabcomponent = go.GetComponent<oo::PrefabComponent>();
	std::string& prefab_data = ImGuiManager::s_prefab_controller->RequestForPrefab((Project::GetPrefabFolder()/prefabcomponent.prefab_filePath).string());
	rapidjson::Document prefab_doc;
	rapidjson::StringStream stream(prefab_data.c_str());
	prefab_doc.ParseStream(stream);
	//+1 to skip the first value
	int child_counter = 0;
	auto childrens = go.GetChildren(true);
	//per child
	for (auto iter_member = prefab_doc.MemberBegin(); iter_member != prefab_doc.MemberEnd(); ++iter_member)
	{
		rapidjson::Value child_value(rapidjson::kObjectType);
		auto orignal_obj = iter_member->value.GetObj();
		SaveObject(childrens[child_counter], child_value, doc);
		std::vector<std::string> component_delete_list;
		//each component
		for (auto iter_childcomponent = child_value.MemberBegin(); iter_childcomponent != child_value.MemberEnd(); ++iter_childcomponent)
		{
			if (orignal_obj.HasMember(iter_childcomponent->name) == false)
			{
				continue;
			}
			auto& orignal_component = orignal_obj.FindMember(iter_childcomponent->name)->value;
			auto& current_component = iter_childcomponent->value;
			//each variable
			std::vector<std::string> delete_list;
			for (auto iter_childVariables = current_component.MemberBegin(); iter_childVariables != current_component.MemberEnd(); ++iter_childVariables)
			{
				std::string debug_stirng = iter_childVariables->name.GetString();
				if (orignal_component.HasMember(iter_childVariables->name) == false)
				{
					continue;
				}
				auto member = orignal_component.FindMember(iter_childVariables->name);
				if (member->value == iter_childVariables->value)
				{
					delete_list.push_back(iter_childVariables->name.GetString());
				}
			}
			if (delete_list.size() == current_component.MemberCount())
				current_component.RemoveAllMembers();
			else
			{
				for (auto iters : delete_list)
					current_component.RemoveMember(iters.c_str());
			}
			//remove the component
			//usually there will be a 100% match which will make child_value empty
			if (current_component.MemberCount() == 0)
			{
				component_delete_list.push_back(iter_childcomponent->name.GetString());
			}
		}
		if (child_value.MemberCount() == component_delete_list.size())
			child_value.RemoveAllMembers();
		else
		{
			for (auto iters : component_delete_list)
				child_value.RemoveMember(iters.c_str());
			rapidjson::Value orignalID(iter_member->name.GetString(),doc.GetAllocator());
			rpj_prefabComponent->value.AddMember(orignalID, child_value, doc.GetAllocator());
		}

		child_counter++;
	}
	//default overwrite
	return;
}

void Serializer::SaveSequentialContainer(rttr::variant variant, rapidjson::Value& val, rttr::property prop,rapidjson::Document& doc)
{
	rapidjson::Value arrayValue(rapidjson::kObjectType);
	rttr::variant_sequential_view sqv = variant.create_sequential_view();

	auto iter_type = UI_RTTRType::types.find(sqv.get_value_type().get_id());
	if (iter_type == UI_RTTRType::types.end())
		return;
	auto sf = m_SaveProperties.m_save_commands.find(iter_type->second);
	if (sf == m_SaveProperties.m_save_commands.end())
		return;

	for (size_t i = 0; i < sqv.get_size(); ++i)
	{
		sf->second(doc,arrayValue, sqv.get_value(i), prop);
	}
	std::string temp = prop.get_name().data();
	rapidjson::Value name;
	name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
	val.AddMember(name, arrayValue, doc.GetAllocator());
}

void Serializer::SaveNestedComponent(rttr::variant var, rapidjson::Value& val, rttr::property _property,rapidjson::Document& doc)
{
	rapidjson::Value sub_component(rapidjson::kObjectType);
	rttr::type type = var.get_type();
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
				rttr::variant variant = prop.get_value(var);
				SaveSequentialContainer(variant, sub_component, prop,doc);
			}
			else if (prop_type.is_class())
			{
				ASSERT_MSG(true, "you are pushing it too far buddy.");
			}
			continue;//not supported
		}
		auto sf = m_SaveProperties.m_save_commands.find(iter->second);
		if (sf == m_SaveProperties.m_save_commands.end())
			continue;//don't have this save function
		sf->second(doc,sub_component, prop.get_value(var), prop);
	}
	std::string temp = _property.get_name().data();
	rapidjson::Value name;
	name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
	val.AddMember(name, sub_component, doc.GetAllocator());
}

UUID Serializer::Loading(std::shared_ptr<oo::GameObject> starting, oo::Scene& scene, rapidjson::Document& doc)
{
	// TODO : This can be improved if required. Clean up please

	UUID firstobj;
	std::stack<std::shared_ptr<oo::GameObject>> parents;
	std::vector<std::shared_ptr<oo::GameObject>> second_iter;
	parents.push(starting);
	for (auto iter = doc.MemberBegin(); iter != doc.MemberEnd(); ++iter)
	{
		uint64_t id = std::stoull(iter->name.GetString());
		auto go = scene.CreateGameObjectImmediate(id);
		auto members = iter->value.MemberBegin();//get the order of hierarchy
		auto membersEnd = iter->value.MemberEnd();
		int order = members->value.GetInt();


		{//when the order dont match the size it will keep poping until it matches
		//then parent to it and adds itself
			while (order != parents.size())
				parents.pop();

			parents.top()->AddChild(*go, true);
			parents.push(go);
			if (iter == doc.MemberBegin())
				firstobj = go->GetInstanceID();
		}


		//++members;
		//{//another element that will store all the component hashes and create the apporiate archtype
		//	// go->SetArchtype(vector<hashes>);
		//}
		////processes the components		
		//LoadObject(*go, members, membersEnd);

		second_iter.emplace_back(go);
	}

	scene.GetWorld().Get_System<oo::TransformSystem>()->UpdateSubTree(*starting, false);

	//std::stack<std::shared_ptr<oo::GameObject>> parents;
	//parents.push(starting);
	int iteration = 0;
	for (auto iter = doc.MemberBegin(); iter != doc.MemberEnd(); ++iter, ++iteration)
	{
		uint64_t id = std::stoull(iter->name.GetString());
		auto go = second_iter[iteration];
		auto members = iter->value.MemberBegin();//get the order of hierarchy
		auto membersEnd = iter->value.MemberEnd();
		int order = members->value.GetInt();

		++members;
		//processes the components		
		LoadObject(*go, members, membersEnd);

		//scene.GetWorld().Get_System<oo::TransformSystem>()->UpdateSubTree(*scene.GetRoot());

		//{//when the order dont match the size it will keep poping until it matches
		////then parent to it and adds itself
		//	while (order != parents.size())
		//		parents.pop();

		//	parents.top()->AddChild(*go, true);
		//	parents.push(go);
		//	if (iter == doc.MemberBegin())
		//		firstobj = go->GetInstanceID();
		//}

		{//another element that will store all the component hashes and create the apporiate archtype
			// go->SetArchtype(vector<hashes>);
		}
	}

	ResetDocument();//clear it after using
	return firstobj;
}

void Serializer::LoadObject(oo::GameObject& go, rapidjson::Value::MemberIterator& iter , rapidjson::Value::MemberIterator& end)
{
	for (; iter != end; ++iter)
	{
		rttr::type t = rttr::type::get_by_name(iter->name.GetString());
		if (t.is_valid() == false)
			continue;//if not valid then skip
		auto lc_iter = load_components.find(t.get_id());
		if (lc_iter == load_components.end())
			continue;//not found
		lc_iter->second(go, std::move(iter->value));
	}
}

void Serializer::LoadSequentialContainer(rttr::variant& variant, rapidjson::Value& val)
{
	rttr::variant_sequential_view sqv = variant.create_sequential_view();

	auto arr_UITypes = UI_RTTRType::types.find(sqv.get_value_type().get_id());
	if (arr_UITypes == UI_RTTRType::types.end())
		return;
	auto command = m_LoadProperties.m_load_commands.find(arr_UITypes->second);
	if (command == m_LoadProperties.m_load_commands.end())
		return;

	size_t size_array = static_cast<size_t>(val.MemberCount());
	sqv.set_size(size_array);
	size_t counter = 0;
	for (auto arrBegin = val.MemberBegin(); arrBegin != val.MemberEnd(); ++arrBegin, ++counter)
	{
		rttr::variant varr = sqv.get_value(counter);
		command->second(varr, std::move(arrBegin->value));
		sqv.set_value(counter, varr);
	}
}

void Serializer::LoadNestedComponent(rttr::variant& variant, rapidjson::Value& val)
{
	for (auto iter = val.MemberBegin(); iter != val.MemberEnd(); ++iter)
	{
		rttr::type t = variant.get_type();
		rttr::property prop = t.get_property(iter->name.GetString());
		if (prop.is_valid() == false)
			continue;

		auto types_UI = UI_RTTRType::types.find(prop.get_type().get_id());
		if (types_UI == UI_RTTRType::types.end())
		{
			rttr::type prop_type = prop.get_type();
			if (prop_type.is_sequential_container())
			{
				rttr::variant v = prop.get_value(variant);
				LoadSequentialContainer(v, iter->value);
				prop.set_value(variant, v);
			}
			else if (prop_type.is_class())
			{
				ASSERT_MSG(true, "you are pushing it too far buddy.");
			}
			continue;//not supported
		}
		auto command = m_LoadProperties.m_load_commands.find(types_UI->second);
		if (command == m_LoadProperties.m_load_commands.end())
			continue;//don't have this save function
		rttr::variant v;
		command->second(v, std::move(iter->value));
		prop.set_value(variant, v);
	}
}

UUID Serializer::CreatePrefab(std::shared_ptr<oo::GameObject> starting, oo::Scene& scene, std::filesystem::path& p)
{
	UUID firstobj;
	
	auto go = scene.CreateGameObjectImmediate();
	go->AddComponent<oo::PrefabComponent>();
	firstobj = go->GetInstanceID();

	starting->AddChild(*go);
	
	if (go->HasComponent<oo::PrefabComponent>() == false)
	{
		std::string msg = "GameObject does not contain this component: ";
		msg += rttr::type::get<oo::PrefabComponent>().get_name().data();
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, msg);
	}
	oo::PrefabComponent& component = go->GetComponent<oo::PrefabComponent>();
	component.prefab_filePath = std::filesystem::relative(p,Project::GetPrefabFolder());

	std::string& data = ImGuiManager::s_prefab_controller->RequestForPrefab((Project::GetPrefabFolder() / component.prefab_filePath).string());
	rapidjson::StringStream stream(data.c_str());

	rapidjson::Document document;
	document.ParseStream(stream);

	std::stack<std::shared_ptr<oo::GameObject>> parents;
	auto gameobj = (go);
	parents.push(gameobj);
	for (auto iter = document.MemberBegin(); iter != document.MemberEnd();)
	{
		//gameobj->SetName(iter->name.GetString());
		gameobj->SetIsPrefab(true);
		auto members = iter->value.MemberBegin();//get the order of hierarchy
		auto membersEnd = iter->value.MemberEnd();
		int order = members->value.GetInt();

		{//when the order dont match the size it will keep poping until it matches
		//then parent to it and adds itself
			while (order != parents.size())
				parents.pop();

			parents.top()->AddChild(*gameobj);
			parents.push(gameobj);
		}

		++members;
		{//another element that will store all the component hashes and create the apporiate archtype
			// go->SetArchtype(vector<hashes>);
		}
		//processes the components		
		LoadObject(*gameobj, members, membersEnd);
		++iter;
		if (iter != document.MemberEnd())
		{
			gameobj = scene.CreateGameObjectImmediate();
		}
	}
	return firstobj;
}

void Serializer::SaveScript(oo::GameObject& go, rapidjson::Value& val, rapidjson::Document& doc)
{
	auto & scriptcomponent = go.GetComponent<oo::ScriptComponent>();
	rapidjson::Value component(rapidjson::kObjectType);
	for (auto& scriptclass : scriptcomponent.GetScriptInfoAll())
	{
		rapidjson::Value value_class(rapidjson::kObjectType);
		for (auto& sfi : scriptclass.second.fieldMap)
		{
			auto iter = m_saveScriptProperties.m_ScriptSave.find(sfi.second.value.GetValueType());
			if (iter == m_saveScriptProperties.m_ScriptSave.end())
				continue;
			iter->second(doc, value_class, sfi.second);
		}
		component.AddMember(rapidjson::Value(scriptclass.first.c_str(),static_cast<rapidjson::SizeType>(scriptclass.first.size()),doc.GetAllocator()), value_class, doc.GetAllocator());
	}
	rapidjson::Value name(scriptcomponent.get_type().get_name().data(), doc.GetAllocator());
	val.AddMember(name, component, doc.GetAllocator());
}

void Serializer::LoadScript(oo::GameObject& go, rapidjson::Value&& scriptComponentValue)
{
	auto& scriptcomponent = go.GetComponent<oo::ScriptComponent>();
	auto val = scriptComponentValue.GetObj();
	
	for (auto iter = val.MemberBegin(); iter != val.MemberEnd(); ++iter)
	{
        try
        {
            std::string script_name = iter->name.GetString();
            auto& scriptInfo = scriptcomponent.AddScriptInfo(oo::ScriptClassInfo{ script_name });

            for (auto classInfoIter = iter->value.MemberBegin(); classInfoIter != iter->value.MemberEnd(); ++classInfoIter)
            {
                auto scriptfieldIter = scriptInfo.fieldMap.find(classInfoIter->name.GetString());
                if (scriptfieldIter == scriptInfo.fieldMap.end())
                    continue;
                auto type = scriptfieldIter->second.value.GetValueType();
                auto sfiLoadIter = m_loadScriptProperties.m_ScriptLoad.find(type);
                if (sfiLoadIter == m_loadScriptProperties.m_ScriptLoad.end())
                    continue;
                sfiLoadIter->second(std::move(classInfoIter->value), scriptfieldIter->second);
            }
        }
        catch (std::exception const& e)
        {
            LOG_ERROR("{0} (DON'T SAVE OR SCRIPTS WILL BE DELETED FROM GAMEOBJECTS)", e.what());
        }
	}
}

void Serializer::ResetDocument() noexcept
{
	//rapidjson::Document d; // new temp document
	//doc.Swap(d).SetObject(); // minimize and recreate allocator
}

