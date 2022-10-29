/************************************************************************************//*!
\file           AnimationGroup.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A collection of nodes and links

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AnimationGroup.h"
#include "AnimationCondition.h"
#include "AnimationInternal.h"

#include <rttr/registration>
namespace oo::Anim::internal
{
	void SerializeGroup(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Group& group)
	{
		writer.StartObject();
		{
			rttr::instance obj{ group };
			//properties
			{
				auto properties = rttr::type::get<Group>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					auto temp = prop.get_type().get_id();
					bool r = internal::serializeDataFn_map.contains(temp);
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}
			//nodes
			writer.Key("Nodes", static_cast<rapidjson::SizeType>(std::string("Nodes").size()));
			writer.StartArray();
			{
				auto serialize_fn = rttr::type::get<Node>().get_method(internal::serialize_method_name);
				for (auto& [id, node] : group.nodes)
					serialize_fn.invoke({}, writer, node);
			}
			writer.EndArray();
			//link
			writer.Key("Links", static_cast<rapidjson::SizeType>(std::string("Links").size()));
			writer.StartArray();
			{
				auto serialize_fn = rttr::type::get<Link>().get_method(internal::serialize_method_name);
				for (auto& [id, link] : group.links)
					serialize_fn.invoke({}, writer, link);
			}
			writer.EndArray();
		}
		writer.EndObject();
	}
}
namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<Group>("Animation Group")
			.property("name", &Group::name)
			.property("startNode", &Group::startNode)
			.property("groupID", &Group::groupID)
			.method(internal::serialize_method_name, &internal::SerializeGroup)
			;
	}

	Group::SerializeFn* const Group::serializeFn = 
	[](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Group& group)
	{
		writer.StartObject();
		//name
		writer.String(group.name.c_str(), static_cast<rapidjson::SizeType>(group.name.size()));
		//id
		writer.Uint64(group.groupID);
		//nodes
		writer.String("Nodes: ", static_cast<rapidjson::SizeType>(std::string("Nodes: ").size()));
		writer.StartArray();
		{
			for(auto& [id, node] : group.nodes)
				Node::serializeFn(writer, node);
		}
		writer.EndArray();
		//link
		writer.Key("Links: ", static_cast<rapidjson::SizeType>(std::string("Links: ").size()));
		writer.StartArray();
		{
			auto serialize_fn = rttr::type::get<Link>().get_method(internal::serialize_method_name);
			for (auto& [id, link] : group.links)
				serialize_fn.invoke({}, writer, link);
		}
		writer.EndArray();
		
		writer.EndObject();
	};

	/*Group::Group(std::string const _name)
		: name{ _name }
		, groupID{ internal::generateUID() }
	{
		NodeInfo info{
			.name{ "Start Node" },
			.animation_name{ Animation::empty_animation_name },
			.speed{ 1.f },
			.position{0.f,0.f,0.f},
			.group{  }
		};

		auto node = Anim::internal::AddNodeToGroup(*this, info);
		startNode = internal::CreateNodeReference(*this, node->node_ID);
	}*/
	Group::Group(GroupInfo const& info)
		: name{ info.name }
		, groupID{ info.groupID == internal::invalid_ID ? internal::generateUID() : info.groupID }
	{
		//verify tree is valid
		//assert(info.tree);


	}
	Group::Group(Group&& other)
	{
		name = std::move(other.name);
		startNode = std::move(other.startNode);
		//tree = std::move(other.tree);
		groupID = std::move(other.groupID);

		nodes.merge(other.nodes);
		links.merge(other.links);

	}
}