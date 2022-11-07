/************************************************************************************//*!
\file           AnimationTree.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
allows you to arrange and maintain a set of Animations and 
Animation Transitions for a gameobject object

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AnimationTree.h"
#include "AnimationParameter.h"
#include "AnimationCondition.h"
#include "AnimationInternal.h"

#include <rttr/registration>
#include <rapidjson/document.h>

namespace oo::Anim::internal
{
	void SerializeTree(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, AnimationTree& tree)
	{
		writer.StartObject();
		{
			rttr::instance obj{ tree };
			//properties
			{
				auto properties = rttr::type::get<AnimationTree>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}
			//groups
			writer.Key("Groups", static_cast<rapidjson::SizeType>(std::string("Groups").size()));
			writer.StartArray();
			{
				auto serialize_fn = rttr::type::get<Group>().get_method(internal::serialize_method_name);
				for (auto& [id, group] : tree.groups)
					serialize_fn.invoke({}, writer, group);
			}
			writer.EndArray();
			//parameters
			writer.Key("Parameters", static_cast<rapidjson::SizeType>(std::string("Parameters").size()));
			writer.StartArray();
			{
				auto serialize_fn = rttr::type::get<Parameter>().get_method(internal::serialize_method_name);
				for (auto& parameter : tree.parameters)
					serialize_fn.invoke({}, writer, parameter);
			}
			writer.EndArray();
		}
		writer.EndObject();
	}

	void LoadTree(rapidjson::GenericObject<false,rapidjson::Value>& object, AnimationTree& tree)
	{
		rttr::instance obj{ tree };
		//properties
		{
			auto properties = rttr::type::get<AnimationTree>().get_properties();
			for (auto& prop : properties)
			{
				auto& value = object.FindMember(prop.get_name().data())->value;
				rttr::variant val{ internal::loadDataFn_map.at(prop.get_type().get_id())(value) };
				prop.set_value(obj, val);
			}
		}
		//groups
		{
			auto groups = object.FindMember("Groups")->value.GetArray();
			auto load_fn = rttr::type::get<Group>().get_method(internal::load_method_name);
			for (auto& group : groups)
			{
				Group new_group{};
				load_fn.invoke({}, group.GetObj(), new_group);

				tree.groups.emplace(new_group.groupID, std::move(new_group));
			}
		}
		
		//parameters
		{
			auto params = object.FindMember("Parameters")->value.GetArray();
			auto load_fn = rttr::type::get<Parameter>().get_method(internal::load_method_name);
			for (auto& param : params)
			{
				Parameter new_param{};
				load_fn.invoke({}, param.GetObj(), new_param);

				tree.parameters.emplace_back(std::move(new_param));
			}
		}
		
		internal::CalculateParameterIndexes(tree);
	}
}
namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<AnimationTree>("Animation Tree")
			.property("name", &AnimationTree::name)
			.property("treeID", &AnimationTree::treeID)
			.method(internal::serialize_method_name, &internal::SerializeTree)
			.method(internal::load_method_name, &internal::LoadTree)
			;
	}


	std::unordered_map<size_t, AnimationTree> AnimationTree::map{};

	AnimationTree* AnimationTree::Create(std::string const name)
	{
		AnimationTree tree{};
		tree.name = name;
		tree.treeID = internal::generateUID();
		auto ptr_to_tree = AnimationTree::Add(std::move(tree));
		assert(ptr_to_tree);
		auto& createdTree = *ptr_to_tree;
		//create a default group and assign to tree
		GroupInfo info{ .name{"Group 1"},.tree{&createdTree} };
		internal::AddGroupToTree(createdTree, info);

		return &createdTree;
	}

	AnimationTree* AnimationTree::Add(AnimationTree&& tree)
	{
		auto key = tree.treeID;
		AnimationTree::map.insert_or_assign(key, std::move(tree));
		return &(AnimationTree::map[key]);
	}
}