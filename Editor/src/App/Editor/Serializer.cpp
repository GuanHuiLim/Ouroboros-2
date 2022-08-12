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
Serializer::Serializer()
{
}

Serializer::~Serializer()
{
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
		doc.Accept(writer);
		rapidjson::Document d; // new temp document
		doc.Swap(d).SetObject(); // minimize and recreate allocator
		ofs.close();
	}
}

void Serializer::SaveObject(oo::GameObject& go, rapidjson::Value& val)
{
	SaveComponent<oo::GameObjectComponent>(go, val);
	bool a = go.HasComponent<oo::GameObjectComponent>();
}

