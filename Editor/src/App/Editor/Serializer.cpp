#include "pch.h"
#include "Serializer.h"

#include <Scenegraph/include/scenenode.h>
#include <Scenegraph/include/Scenegraph.h>
#include <SceneManagement/include/SceneManager.h>

#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Prefab/PrefabComponent.h>

#include <Ouroboros/ECS/GameObjectComponent.h>
#include <Ouroboros/EventSystem/EventManager.h>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "Project.h"

#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Events/LoadSceneEvent.h"
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
	doc.SetObject();

	save_commands.emplace(UI_RTTRType::UItypes::BOOL_TYPE, [](rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		obj.AddMember(name, rapidjson::Value(variant.get_value<bool>()), doc.GetAllocator());
		});
	save_commands.emplace(UI_RTTRType::UItypes::STRING_TYPE, [](rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value v;
		std::string val = variant.to_string();
		v.SetString(val.c_str(), static_cast<rapidjson::SizeType>(val.size()), doc.GetAllocator());
		obj.AddMember(name , v, doc.GetAllocator());
		});
	save_commands.emplace(UI_RTTRType::UItypes::PATH_TYPE, [](rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value v;
		std::string val = variant.get_value<std::filesystem::path>().string();
		v.SetString(val.c_str(), static_cast<rapidjson::SizeType>(val.size()), doc.GetAllocator());
		obj.AddMember(name, v, doc.GetAllocator());
		});

	AddLoadComponent<oo::GameObjectComponent>();
	AddLoadComponent<oo::TransformComponent>();
	AddLoadComponent<oo::PrefabComponent>();

	load_commands.emplace(UI_RTTRType::UItypes::BOOL_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetBool();});
	load_commands.emplace(UI_RTTRType::UItypes::STRING_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = static_cast<std::string>(val.GetString());});
	load_commands.emplace(UI_RTTRType::UItypes::PATH_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetString(); });
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

	Saving(s,parents,scene);
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
	
	doc.ParseStream(isw);
	Loading(scene.GetRoot(),scene);
	ifs.close();
}

std::filesystem::path Serializer::SavePrefab(std::shared_ptr<oo::GameObject> go , oo::Scene & scene)
{
	std::stack<scenenode::raw_pointer> s;
	std::stack<scenenode::handle_type> parents;
	scenenode::raw_pointer curr = (*go).GetSceneNode().lock().get();
	parents.push(curr->get_handle());
	s.push(curr);
	Saving(s,parents, scene);
	std::filesystem::path newprefabPath = Project::GetPrefabFolder().string() + go->Name() + ".prefab";
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

void Serializer::LoadPrefab(std::filesystem::path path, std::shared_ptr<oo::GameObject> go)
{
	//query the item from the prefab scene
	//copy the item to go
}

std::string Serializer::SaveDeletedObject(std::shared_ptr<oo::GameObject> go, oo::Scene& scene)
{
	std::stack<scenenode::raw_pointer> s;
	std::stack<scenenode::handle_type> parents;
	scenenode::raw_pointer curr = (*go).GetSceneNode().lock().get();
	parents.push(curr->get_handle());
	s.push(curr);
	Saving(s,parents, scene);
	rapidjson::StringBuffer buffer(0,512);
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	std::string temp = buffer.GetString();
	ResetDocument();
	return temp;
}

UUID Serializer::LoadDeleteObject(std::string& data, UUID parentID, oo::Scene& scene)
{
	rapidjson::StringStream stream(data.c_str());
	doc.ParseStream(stream);
	auto parent = scene.FindWithInstanceID(parentID);

	ASSERT_MSG(parent == nullptr, "parent not found");
	
	auto firstObj = Loading(parent,scene);
	ResetDocument();
	return firstObj;
}

void Serializer::Saving(std::stack<scenenode::raw_pointer>& s, std::stack<scenenode::handle_type>& parents, oo::Scene& scene)
{


	scenenode::raw_pointer curr ;
	while (!s.empty())
	{
		curr = s.top();
		s.pop();
		auto go = scene.FindWithInstanceID(curr->get_handle());
		//bulk of code here
		rapidjson::Value name;
		name.SetString(go->Name().c_str(), static_cast<rapidjson::SizeType>(go->Name().size()));

		rapidjson::Value gameobject_start;
		gameobject_start.SetObject();
		gameobject_start.AddMember("Order", parents.size(), doc.GetAllocator());

		bool is_prefab = go->HasComponent<oo::PrefabComponent>();
		is_prefab ? SavePrefabObject(*go, gameobject_start) : SaveObject(*go, gameobject_start);


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

void Serializer::SaveObject(oo::GameObject& go, rapidjson::Value& val)
{
	//will have more components
	SaveComponent<oo::GameObjectComponent>(go, val);
	SaveComponent<oo::TransformComponent>(go, val);
}

void Serializer::SavePrefabObject(oo::GameObject& go, rapidjson::Value& val)
{
	SaveComponent<oo::PrefabComponent>(go, val);
	SaveComponent<oo::GameObjectComponent>(go, val);
	SaveComponent<oo::TransformComponent>(go, val);
	return;
}

void Serializer::SaveSequentialContainer(rttr::variant variant, rapidjson::Value& val, rttr::property prop)
{
	rapidjson::Value arrayValue(rapidjson::kObjectType);
	rttr::variant_sequential_view sqv = variant.create_sequential_view();

	auto iter_type = UI_RTTRType::types.find(sqv.get_value_type().get_id());
	if (iter_type == UI_RTTRType::types.end())
		return;
	auto sf = save_commands.find(iter_type->second);
	if (sf == save_commands.end())
		return;

	for (size_t i = 0; i < sqv.get_size(); ++i)
	{
		sf->second(arrayValue, sqv.get_value(i), prop);
	}
	std::string temp = prop.get_name().data();
	rapidjson::Value name;
	name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
	val.AddMember(name, arrayValue, doc.GetAllocator());
}

void Serializer::SaveNestedComponent(rttr::variant var, rapidjson::Value& val, rttr::property _property)
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
				SaveSequentialContainer(variant, sub_component, prop);
			}
			else if (prop_type.is_class())
			{
				ASSERT_MSG(true, "you are pushing it too far buddy.");
			}
			continue;//not supported
		}
		auto sf = save_commands.find(iter->second);
		if (sf == save_commands.end())
			continue;//don't have this save function
		sf->second(sub_component, prop.get_value(var), prop);
	}
	std::string temp = _property.get_name().data();
	rapidjson::Value name;
	name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
	val.AddMember(name, sub_component, doc.GetAllocator());
}

UUID Serializer::Loading(std::shared_ptr<oo::GameObject> starting, oo::Scene& scene)
{
	UUID firstobj;
	std::stack<std::shared_ptr<oo::GameObject>> parents;
	parents.push(starting);
	for (auto iter = doc.MemberBegin(); iter != doc.MemberEnd(); ++iter)
	{
		auto go = scene.CreateGameObjectImmediate();
		go->SetName(iter->name.GetString());
		auto members = iter->value.MemberBegin();//get the order of hierarchy
		auto membersEnd = iter->value.MemberEnd();
		int order = members->value.GetInt();

		{//when the order dont match the size it will keep poping until it matches
		//then parent to it and adds itself
			while (order != parents.size())
				parents.pop();

			parents.top()->AddChild(*go);
			parents.push(go);
			if (iter == doc.MemberBegin())
				firstobj = go->GetInstanceID();
		}

		++members;
		{//another element that will store all the component hashes and create the apporiate archtype
			// go->SetArchtype(vector<hashes>);
		}
		//processes the components		
		LoadObject(*go, members, membersEnd);
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
	auto command = load_commands.find(arr_UITypes->second);
	if (command == load_commands.end())
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
		auto command = load_commands.find(types_UI->second);
		if (command == load_commands.end())
			continue;//don't have this save function
		rttr::variant v;
		command->second(v, std::move(iter->value));
		prop.set_value(variant, v);
	}
}

void Serializer::ResetDocument() noexcept
{
	rapidjson::Document d; // new temp document
	doc.Swap(d).SetObject(); // minimize and recreate allocator
}

