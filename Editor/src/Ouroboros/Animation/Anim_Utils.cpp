/************************************************************************************//*!
\file           Anim_Utils.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          Utility definitions for Animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Anim_Utils.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "AnimationNode.h"
#include "AnimationCondition.h"
#include "Animation.h"
#include "AnimationKeyFrame.h"

#include <rttr/instance.h>
#include <rttr/registration>

#include <rapidjson/document.h>

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::enumeration<oo::Anim::P_TYPE>("Parameter Type")
	(
		value("BOOL", oo::Anim::P_TYPE::BOOL),
		value("TRIGGER", oo::Anim::P_TYPE::TRIGGER),
		value("INT", oo::Anim::P_TYPE::INT),
		value("FLOAT", oo::Anim::P_TYPE::FLOAT)
	)
	;

	
}
namespace oo::Anim::internal
{
	template<typename T>
	auto assign_to_map = [](std::unordered_map< size_t, rttr::instance(*)(void*)>& map) {
		//map.emplace(, &conversion_fn<T>);

		map[Ecs::ECSWorld::get_component_hash<T>()] = [](void* ptr) {
			T& ref = *(static_cast<T*>(ptr));
			return rttr::instance{ ref };
		};
	};
	std::unordered_map< size_t, rttr::instance(*)(void*)> hash_to_instance{};

	void Initialise_hash_to_instance()
	{
		if (hash_to_instance.empty() == false) return;

		assign_to_map<TransformComponent>(hash_to_instance);

		/*hash_to_instance =
			[]() {
			std::unordered_map< size_t, rttr::instance(*)(void*)> map;

			assign_to_map<TransformComponent>(map);

			return map;
		}();*/
	}

	size_t generateUID()
	{
		static std::mt19937_64 mt{ std::random_device{}() };
		static std::uniform_int_distribution<size_t> distrib{ 0 };
		return distrib(mt);
	};
}

namespace oo::Anim
{

	


	/*Parameter::SerializeFnMap const Parameter::serializeFn_map = 
		[]() {
		Parameter::SerializeFnMap map;
		
		map[P_TYPE::BOOL] =
		[](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Parameter& param)
		{
			writer.String("BOOL", static_cast<rapidjson::SizeType>(std::string("BOOL").size()));
			writer.Bool(param.value.get_value<bool>());
		};

		map[P_TYPE::TRIGGER] =
			[](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Parameter& param)
		{
			writer.String("TRIGGER", static_cast<rapidjson::SizeType>(std::string("TRIGGER").size()));
			writer.Bool(param.value.get_value<bool>());
		};

		map[P_TYPE::FLOAT] =
			[](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Parameter& param)
		{
			writer.String("FLOAT", static_cast<rapidjson::SizeType>(std::string("FLOAT").size()));
			writer.Double(static_cast<double>(param.value.get_value<float>()));
		};

		map[P_TYPE::INT] =
			[](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Parameter& param)
		{
			writer.String("INT", static_cast<rapidjson::SizeType>(std::string("INT").size()));
			writer.Int(param.value.get_value<int>());
		};

		return map;
		}();*/

}

namespace oo::Anim
{
	/*------------------------
	serialize
	------------------------*/
	void SerializeGroupRef(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, GroupRef& ref)
	{
		writer.StartObject();
		if (ref)
		{
			writer.Key("GroupRef", static_cast<rapidjson::SizeType>(std::string("GroupRef").size()));
			writer.String(ref->name.c_str(), static_cast<rapidjson::SizeType>(ref->name.size()));
		}
		else
		{
			LOG_CORE_DEBUG_CRITICAL("Group reference {0} invalid when serializing!!", ref.id);
			assert(false);
		}
		writer.Key("ID", static_cast<rapidjson::SizeType>(std::string("ID").size()));
		writer.Uint64(ref.id);
		writer.EndObject();
	}
	void SerializeNodeRef(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, NodeRef& ref)
	{
		writer.StartObject();
		if (ref)
		{
			writer.Key("NodeRef", static_cast<rapidjson::SizeType>(std::string("NodeRef").size()));
			writer.String(ref->name.c_str(), static_cast<rapidjson::SizeType>(ref->name.size()));
		}
		else
		{
			LOG_CORE_DEBUG_CRITICAL("Node reference {0} invalid when serializing!!", ref.id);
			assert(false);
		}
		writer.Key("ID", static_cast<rapidjson::SizeType>(std::string("ID").size()));
		writer.Uint64(ref.id);
		writer.EndObject();
	}
	void SerializeLinkRef(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, LinkRef& ref)
	{
		writer.StartObject();
		if (ref)
		{
			writer.Key("LinkRef", static_cast<rapidjson::SizeType>(std::string("LinkRef").size()));
			writer.String(ref->name.c_str(), static_cast<rapidjson::SizeType>(ref->name.size()));
		}
		else
		{
			LOG_CORE_DEBUG_CRITICAL("Link reference {0} invalid when serializing!!", ref.id);
			assert(false);
		}
		writer.Key("ID", static_cast<rapidjson::SizeType>(std::string("ID").size()));
		writer.Uint64(ref.id);
		writer.EndObject();
	}
	void SerializeAnimRef(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, AnimRef& ref)
	{
		writer.StartObject();
		if (ref)
		{
			writer.Key("AnimRef", static_cast<rapidjson::SizeType>(std::string("AnimRef").size()));
			writer.String(ref->name.c_str(), static_cast<rapidjson::SizeType>(ref->name.size()));
		}
		else
		{
			LOG_CORE_DEBUG_CRITICAL("Anim reference {0} invalid when serializing!!", ref.id);
			assert(false);
		}
		writer.Key("ID", static_cast<rapidjson::SizeType>(std::string("ID").size()));
		writer.Uint64(ref.id);
		writer.EndObject();
	}
	/*------------------------
	load
	------------------------*/
	void LoadGroupRef(rapidjson::Value& value, GroupRef& ref)
	{
		auto obj = value.GetObj();
		ref.id = obj.FindMember("ID")->value.GetUint64();
	}
	void LoadNodeRef(rapidjson::Value& value, NodeRef& ref)
	{
		auto obj = value.GetObj();
		ref.id = obj.FindMember("ID")->value.GetUint64();
	}
	void LoadLinkRef(rapidjson::Value& value, LinkRef& ref)
	{
		auto obj = value.GetObj();
		auto test = obj.FindMember("ID")->value.GetUint64();
		ref.id = obj.FindMember("ID")->value.GetUint64();
	}
	void LoadAnimRef(rapidjson::Value& value, AnimRef& ref)
	{
		auto obj = value.GetObj();
		ref.id = obj.FindMember("ID")->value.GetUint64(); 
	}
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<GroupRef>("Animation Group Reference")
			.property("ID", &GroupRef::id)
			.method(internal::serialize_method_name,&SerializeGroupRef)
			.method(internal::load_method_name,&LoadGroupRef)
			;
		registration::class_<NodeRef>("Animation Node Reference")
			.property("ID", &NodeRef::id)
			.method(internal::serialize_method_name, &SerializeNodeRef)
			.method(internal::load_method_name, &LoadNodeRef)
			;
		registration::class_<LinkRef>("Animation Link Reference")
			.property("ID", &LinkRef::id)
			.method(internal::serialize_method_name, &SerializeLinkRef)
			.method(internal::load_method_name, &LoadLinkRef)
			;
		registration::class_<AnimRef>("Animation Anim Reference")
			.property("ID", &AnimRef::id)
			.method(internal::serialize_method_name, &SerializeAnimRef)
			.method(internal::load_method_name, &LoadAnimRef)
			;
	};
	GroupRef::GroupRef(std::map<size_t, Group>* _groups, size_t _id) :
		groups{ _groups },
		id{ _id }
	{

	}
	void GroupRef::Reload()
	{
		assert(groups);
		assert((*groups).contains(id));
		/*int currindex{ 0 };
		for (auto& group : *groups)
		{
			if (group.groupID == id)
			{
				index = currindex;
				return;
			}
			++currindex;
		}

		LOG_CORE_ERROR("Animation Group Reference Reload failed!!");
		assert(false);
		index = -1;*/
	}
	bool GroupRef::valid() const
	{
		/*return groups && index >= 0 && index < groups->size() &&
			this->operator->()->groupID == id;*/
		assert(groups);
		return (*groups).contains(id);
	}
	Group& GroupRef::Retrieve(size_t id) const
	{
		return (*groups).at(id);
	}
	/*-------------------------------
	NodeRef
	-------------------------------*/
	NodeRef::NodeRef(std::map<size_t, Node>* _nodes, size_t _id) :
		nodes{ _nodes },
		id{ _id }
	{

	}

	void NodeRef::Reload()
	{
		assert(nodes);
		assert((*nodes).contains(id));
		/*int currindex{0};
		for (auto& [node_id, node] : *nodes)
		{
			if (node.node_ID == id)
			{
				index = currindex;
				return;
			}
			++currindex;
		}

		LOG_CORE_ERROR("Animation Node Reference Reload failed!!");
		assert(false);
		index = -1;*/
	}
	bool NodeRef::valid() const
	{
		/*return nodes && index >= 0 && index < nodes->size() &&
			this->operator->()->node_ID == id;*/
		assert(nodes);
		return (*nodes).contains(id);
	}
	Node& NodeRef::Retrieve(size_t id) const
	{
		return (*nodes).at(id);
	}
	/*-------------------------------
	LinkRef
	-------------------------------*/
	LinkRef::LinkRef(std::map<size_t, Link>* _links, size_t _id) :
		links{ _links },
		id{ _id }
	{

	}
	void LinkRef::Reload()
	{
		assert(links);
		assert((*links).contains(id));
		/*int currindex{ 0 };
		for (auto& link : *links)
		{
			if (link.linkID == id)
			{
				index = currindex;
				return;
			}
			++currindex;
		}

		LOG_CORE_ERROR("Animation Link Reference Reload failed!!");
		assert(false);
		index = -1;*/
	}
	bool LinkRef::valid() const
	{
		/*return links && index >= 0 && index < links->size() &&
			this->operator->()->linkID == id;*/
		assert(links);
		return (*links).contains(id);
	}
	Link& LinkRef::Retrieve(size_t id) const
	{
		return (*links).at(id);
	}
	/*-------------------------------
	AnimRef
	-------------------------------*/
	void AnimRef::Reload()
	{
		assert(anims);
		assert((*anims).contains(id));
		/*int currindex{ 0 };
		for (auto& anim : *anims)
		{
			if (anim.animation_ID == id)
			{
				index = currindex;
				return;
			}
			++currindex;
		}

		LOG_CORE_ERROR("Animation Reference Reload failed!!");
		assert(false);
		index = -1;*/
	}
	bool AnimRef::valid() const
	{
		/*return anims && index >= 0 && index < anims->size() &&
			this->operator->()->animation_ID == id;*/
		return (*anims).contains(id);
	}
	Animation& AnimRef::Retrieve(size_t id) const
	{
		return (*anims).at(id);
	}
}