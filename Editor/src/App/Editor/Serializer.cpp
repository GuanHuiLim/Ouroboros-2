#include "pch.h"
#include "Serializer.h"

#include <Scenegraph/include/scenenode.h>
#include <Scenegraph/include/Scenegraph.h>
#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/ECS/GameObject.h>

#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>//rapidjson
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

#include <Ouroboros/ECS/GameObjectComponent.h>
#include <Ouroboros/EventSystem/EventManager.h>
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
		std::string val = variant.get_value<std::string>();
		v.SetString(val.c_str(), static_cast<rapidjson::SizeType>(val.size()), doc.GetAllocator());
		obj.AddMember(name , v, doc.GetAllocator());
		});

	AddLoadComponent<oo::GameObjectComponent>();

	load_commands.emplace(UI_RTTRType::UItypes::BOOL_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetBool();});
	load_commands.emplace(UI_RTTRType::UItypes::STRING_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetString(); });
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
		SaveObject(*go, gameobject_start);

		
		doc.AddMember(name, gameobject_start,doc.GetAllocator());
		//end of writing this gameobject
		if (curr->get_direct_child_count())//if there is childs
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

	std::ofstream ofs(scene.GetFilePath());
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
	std::stack<std::shared_ptr<oo::GameObject>> parents;
	parents.push(scene.GetRoot());
	for (auto iter = doc.MemberBegin(); iter != doc.MemberEnd(); ++iter)
	{
		auto go = scene.CreateGameObject();
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
		}

		++members;
		{//another element that will store all the component hashes and create the apporiate archtype
			// go->SetArchtype(vector<hashes>);
		}
		//processes the components		
		LoadObject(*go, members, membersEnd);
	}
	ResetDocument();//clear it after using
	ifs.close();
}

void Serializer::SaveObject(oo::GameObject& go, rapidjson::Value& val)
{
	//will have more components
	SaveComponent<oo::GameObjectComponent>(go, val);
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

void Serializer::ResetDocument() noexcept
{
	rapidjson::Document d; // new temp document
	doc.Swap(d).SetObject(); // minimize and recreate allocator
}

