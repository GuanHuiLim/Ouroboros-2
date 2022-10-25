/************************************************************************************//*!
\file           AnimationNode.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A basic building block of an animation tree which references an animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AnimationNode.h"
#include "AnimationInternal.h"

#include <rttr/registration>
namespace oo::Anim::internal
{
	void SerializeNode(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Node& node)
	{
		writer.StartObject();
		{
			rttr::instance obj{ node };
			//properties
			{
				auto properties = rttr::type::get<Node>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}
			//outgoing links
			{
				auto method = rttr::type::get<LinkRef>().get_method(internal::serialize_method_name);
				writer.Key("Outgoing Links", static_cast<rapidjson::SizeType>(std::string("Outgoing Links").size()));
				writer.StartArray();
				for (auto& link : node.outgoingLinks)
					method.invoke({}, writer, link);
				writer.EndArray();
			}
		}
		writer.EndObject();
	}
}
namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<Node>("Animation Node")
			.property("group", &Node::group)
			.property("name", &Node::name)
			.property("anim", &Node::anim)
			.property("speed", &Node::speed)
			.property("position", &Node::position)
			.property("node_ID", &Node::node_ID)
			.method(internal::serialize_method_name, &internal::SerializeNode)
			;
	}

	Node::SerializeFn* const Node::serializeFn =
		[](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Node& node)
	{
		writer.StartArray();
		{
			//name
			writer.String(node.name.c_str(), static_cast<rapidjson::SizeType>(node.name.size()));
			//id
			writer.Uint64(node.node_ID);
			//animation
			writer.String(node.GetAnimation().name.c_str(), static_cast<rapidjson::SizeType>(node.GetAnimation().name.size()));
			//outgoing links
			writer.String("Outgoing Links: ", static_cast<rapidjson::SizeType>(std::string("Outgoing Links: ").size()));
			writer.StartArray();
			{
				for (auto& link : node.outgoingLinks)
				{
					writer.String(link->name.c_str(), static_cast<rapidjson::SizeType>(link->name.size()));
				}
			}
			writer.EndArray();
			//speed
			writer.Double(static_cast<double>(node.speed));
			//position
			writer.String("Position: ", static_cast<rapidjson::SizeType>(std::string("Position: ").size()));
			writer.StartArray();
			{
				writer.Double(static_cast<double>(node.position.x));
				writer.Double(static_cast<double>(node.position.y));
				writer.Double(static_cast<double>(node.position.z));
			}
			writer.EndArray();
		}
		writer.EndArray();
	};

	/*Node::Node(Group& _group, std::string const _name)
		: group{ _group },
		name{ _name }
	{

	}*/

	Node::Node(NodeInfo& info)
		: group{ (assert(info.group), info.group) }
		, name{ info.name }
		, node_ID{ info.nodeID == internal::invalid_ID ? internal::generateUID() : info.nodeID }
	{
		assert(Animation::name_to_ID.contains(info.animation_name));
		anim.anims = &(Animation::animation_storage);
		anim.id = Animation::animation_storage[
			Animation::name_to_ID[info.animation_name]].animation_ID;
		anim.Reload();
	}


	Animation& Node::GetAnimation()
	{
		assert(anim);
		return *anim;
	}
}