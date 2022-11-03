/************************************************************************************//*!
\file           AnimationInternal.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
Internal functions used by the animation system implementation. Not for external use.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"
#include "AnimationInternal.h"
#include "AnimationGroup.h"
#include "AnimationNode.h"
#include "AnimationLink.h"
#include "AnimationCondition.h"
#include "AnimationTimeline.h"
#include "Animation.h"
#include "AnimationTree.h"
#include "AnimationTracker.h"
#include "AnimationSystem.h"
#include "AnimationComponent.h"

std::unordered_map <StringHash::size_type, rttr::type> oo::Anim::internal::hash_to_rttrType{};
std::unordered_map <rttr::type::type_id, StringHash::size_type> oo::Anim::internal::rttrType_to_hash = []()
{
	std::unordered_map <rttr::type::type_id, StringHash::size_type> map;
	auto add = [&]( rttr::type type, std::string const str) {
		auto hash = StringHash::GenerateFNV1aHash(str);
		map[type.get_id()] = hash;
		oo::Anim::internal::hash_to_rttrType.emplace(hash, type);
	};

	add(rttr::type::get<bool>(), "bool");
	add(rttr::type::get<float>(), "float");
	add(rttr::type::get<int>(), "int");
	add(rttr::type::get<size_t>(), "size_t");
	add(rttr::type::get<std::string>(), "std::string");
	add(rttr::type::get<glm::vec3>(), "glm::vec3");
	add(rttr::type::get<glm::quat>(), "glm::quat");

	return map;
}();

std::unordered_map<rttr::type::type_id, oo::Anim::internal::SerializeFn*> oo::Anim::internal::serializeDataFn_map = []()
{
	std::unordered_map<rttr::type::type_id, SerializeFn*> map;

	//built in types
	map[rttr::type::get<bool>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.Bool(val.get_value<bool>());
	};
	map[rttr::type::get<float>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.Double(static_cast<double>(val.get_value<float>()));
	};
	map[rttr::type::get<int>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.Int(val.get_value<int>());
	};
	map[rttr::type::get<size_t>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.Uint64(val.get_value<size_t>());
	};

	//rttr types
	map[rttr::type::get<rttr::variant>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto name = val.get_type().get_name();
		assert(serializeRTTRVariantFn_map.contains(val.get_type().get_id()));
		serializeRTTRVariantFn_map[val.get_type().get_id()](writer, val);
	};
	map[rttr::type::get<rttr::type>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto& type = val.get_value<rttr::type>();
		writer.String(type.get_name().data(), static_cast<rapidjson::SizeType>(type.get_name().size()));
	};
	map[rttr::type::get<rttr::property>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto& prop = val.get_value<rttr::property>();
		writer.StartObject();
		writer.Key("Type", static_cast<rapidjson::SizeType>(std::string("Type").size()));
		rttr::variant type = prop.get_type();
		oo::Anim::internal::serializeDataFn_map.at(rttr::type::get<rttr::type>().get_id())(writer, type);

		writer.Key("Property", static_cast<rapidjson::SizeType>(std::string("Property").size()));
		writer.String(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
		writer.EndObject();
	};

	//complex types
	map[rttr::type::get<glm::vec3>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto& vec = val.get_value<glm::vec3>();
		writer.StartObject();
		{
			writer.Key("X", static_cast<rapidjson::SizeType>(std::string("X").size()));
			rttr::variant x = vec.x;
			oo::Anim::internal::serializeDataFn_map.at(rttr::type::get<float>().get_id())(writer, x);
		}
		{
			writer.Key("Y", static_cast<rapidjson::SizeType>(std::string("Y").size()));
			rttr::variant y = vec.y;
			oo::Anim::internal::serializeDataFn_map.at(rttr::type::get<float>().get_id())(writer, y);
		}
		{
			writer.Key("Z", static_cast<rapidjson::SizeType>(std::string("Z").size()));
			rttr::variant z = vec.z;
			oo::Anim::internal::serializeDataFn_map.at(rttr::type::get<float>().get_id())(writer, z);
		}
		writer.EndObject();
	};

	map[rttr::type::get<glm::quat>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto& quaternion = val.get_value<glm::quat>();
		writer.StartObject();
		{
			writer.Key("X", static_cast<rapidjson::SizeType>(std::string("X").size()));
			rttr::variant x = quaternion.x;
			oo::Anim::internal::serializeDataFn_map.at(rttr::type::get<float>().get_id())(writer, x);
		}
		{
			writer.Key("Y", static_cast<rapidjson::SizeType>(std::string("Y").size()));
			rttr::variant y = quaternion.y;
			oo::Anim::internal::serializeDataFn_map.at(rttr::type::get<float>().get_id())(writer, y);
		}
		{
			writer.Key("Z", static_cast<rapidjson::SizeType>(std::string("Z").size()));
			rttr::variant z = quaternion.z;
			oo::Anim::internal::serializeDataFn_map.at(rttr::type::get<float>().get_id())(writer, z);
		}
		{
			writer.Key("W", static_cast<rapidjson::SizeType>(std::string("W").size()));
			rttr::variant w = quaternion.w;
			oo::Anim::internal::serializeDataFn_map.at(rttr::type::get<float>().get_id())(writer, w);
		}
		writer.EndObject();
	};

	map[rttr::type::get<std::string>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto& str = val.get_value<std::string>();
		writer.String(str.c_str(), static_cast<rapidjson::SizeType>(str.size()));
	};
	map[rttr::type::get<Condition::CompareType>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto str = rttr::type::get <Condition::CompareType>().get_enumeration().value_to_name(val);
		writer.String(str.data(), static_cast<rapidjson::SizeType>(str.size()));
	};
	map[rttr::type::get<oo::Anim::P_TYPE>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto str = rttr::type::get <oo::Anim::P_TYPE>().get_enumeration().value_to_name(val);
		writer.String(str.data(), static_cast<rapidjson::SizeType>(str.size()));
	};
	map[rttr::type::get<oo::Anim::Timeline::TYPE>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto str = rttr::type::get <oo::Anim::Timeline::TYPE>().get_enumeration().value_to_name(val);
		writer.String(str.data(), static_cast<rapidjson::SizeType>(str.size()));
	};
	map[rttr::type::get<oo::Anim::Timeline::DATATYPE>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto str = rttr::type::get <oo::Anim::Timeline::DATATYPE>().get_enumeration().value_to_name(val);
		writer.String(str.data(), static_cast<rapidjson::SizeType>(str.size()));
	};
	map[rttr::type::get<oo::ScriptValue::function_info>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto& scriptinfo = val.get_value< oo::ScriptValue::function_info>();
		writer.StartObject();
		writer.Key("classNamespace", static_cast<rapidjson::SizeType>(std::string("classNamespace").size()));
		writer.String(scriptinfo.classNamespace.c_str(), static_cast<rapidjson::SizeType>(scriptinfo.classNamespace.size()));

		writer.Key("className", static_cast<rapidjson::SizeType>(std::string("className").size()));
		writer.String(scriptinfo.className.c_str(), static_cast<rapidjson::SizeType>(scriptinfo.className.size()));

		writer.Key("functionName", static_cast<rapidjson::SizeType>(std::string("functionName").size()));
		writer.String(scriptinfo.functionName.c_str(), static_cast<rapidjson::SizeType>(scriptinfo.functionName.size()));

		//TODO: set up rttr registration for ScriptValue::function_info so I dont have to deal with this
		//writer.Key("paramList", static_cast<rapidjson::SizeType>(std::string("paramList").size()));
		writer.EndObject();
	};
	//references
	map[rttr::type::get<GroupRef>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto serialize_fn = rttr::type::get<GroupRef>().get_method(internal::serialize_method_name);
		serialize_fn.invoke({}, writer, val.get_value<GroupRef>());
	};
	map[rttr::type::get<NodeRef>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto serialize_fn = rttr::type::get<NodeRef>().get_method(internal::serialize_method_name);
		serialize_fn.invoke({}, writer, val.get_value<NodeRef>());
	};
	map[rttr::type::get<LinkRef>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto serialize_fn = rttr::type::get<LinkRef>().get_method(internal::serialize_method_name);
		serialize_fn.invoke({}, writer, val.get_value<LinkRef>());
	};
	map[rttr::type::get<AnimRef>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		auto serialize_fn = rttr::type::get<AnimRef>().get_method(internal::serialize_method_name);
		serialize_fn.invoke({}, writer, val.get_value<AnimRef>());
	};
	return map;
}();

std::unordered_map<rttr::type::type_id, oo::Anim::internal::SerializeFn*> oo::Anim::internal::serializeRTTRVariantFn_map = []()
{
	std::unordered_map<rttr::type::type_id, SerializeFn*> map;

	map[rttr::type::get<bool>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.StartObject();
		writer.Key("Type Hash", static_cast<rapidjson::SizeType>(std::string("Type Hash").size()));
		writer.Uint64(static_cast<uint64_t>(rttrType_to_hash[rttr::type::get<bool>().get_id()]));
		writer.Key("Value", static_cast<rapidjson::SizeType>(std::string("Value").size()));
		writer.Bool(val.get_value<bool>());
		writer.EndObject();
	};
	map[rttr::type::get<float>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.StartObject();
		writer.Key("Type Hash", static_cast<rapidjson::SizeType>(std::string("Type Hash").size()));
		writer.Uint64(static_cast<uint64_t>(rttrType_to_hash[rttr::type::get<float>().get_id()]));
		writer.Key("Value", static_cast<rapidjson::SizeType>(std::string("Value").size()));
		writer.Double(static_cast<double>(val.get_value<float>()));
		writer.EndObject();
	};
	map[rttr::type::get<int>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.StartObject();
		writer.Key("Type Hash", static_cast<rapidjson::SizeType>(std::string("Type Hash").size()));
		writer.Uint64(static_cast<uint64_t>(rttrType_to_hash[rttr::type::get<int>().get_id()]));
		writer.Key("Value", static_cast<rapidjson::SizeType>(std::string("Value").size()));
		writer.Int(val.get_value<int>());
		writer.EndObject();
	};
	map[rttr::type::get<size_t>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.StartObject();
		writer.Key("Type Hash", static_cast<rapidjson::SizeType>(std::string("Type Hash").size()));
		writer.Uint64(static_cast<uint64_t>(rttrType_to_hash[rttr::type::get<size_t>().get_id()]));
		writer.Key("Value", static_cast<rapidjson::SizeType>(std::string("Value").size()));
		writer.Uint64(val.get_value<size_t>());
		writer.EndObject();
	};
	map[rttr::type::get<std::string>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.StartObject();
		writer.Key("Type Hash", static_cast<rapidjson::SizeType>(std::string("Type Hash").size()));
		writer.Uint64(static_cast<uint64_t>(rttrType_to_hash[rttr::type::get<std::string>().get_id()]));
		writer.Key("Value", static_cast<rapidjson::SizeType>(std::string("Value").size()));
		auto& str = val.get_value<std::string>();
		writer.String(str.c_str(), static_cast<rapidjson::SizeType>(str.size()));
		writer.EndObject();
	};

	map[rttr::type::get<glm::vec3>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.StartObject();
		writer.Key("Type Hash", static_cast<rapidjson::SizeType>(std::string("Type Hash").size()));
		writer.Uint64(static_cast<uint64_t>(rttrType_to_hash[rttr::type::get<glm::vec3>().get_id()]));
		writer.Key("Value", static_cast<rapidjson::SizeType>(std::string("Value").size()));
		internal::serializeDataFn_map.at(val.get_type().get_id())(writer, val);
		writer.EndObject();
	};

	map[rttr::type::get<glm::quat>().get_id()]
		= [](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, rttr::variant& val)
	{
		writer.StartObject();
		writer.Key("Type Hash", static_cast<rapidjson::SizeType>(std::string("Type Hash").size()));
		writer.Uint64(static_cast<uint64_t>(rttrType_to_hash[rttr::type::get<glm::vec3>().get_id()]));
		writer.Key("Value", static_cast<rapidjson::SizeType>(std::string("Value").size()));
		internal::serializeDataFn_map.at(val.get_type().get_id())(writer, val);
		writer.EndObject();
	};
	return map;
}();

std::unordered_map<rttr::type::type_id, oo::Anim::internal::LoadFn*> oo::Anim::internal::loadDataFn_map = []()
{
	std::unordered_map<rttr::type::type_id, oo::Anim::internal::LoadFn*> map;
	/*------------------------
	built in types
	------------------------*/
	map[rttr::type::get<bool>().get_id()]
		= [](rapidjson::Value& value)
	{
		return rttr::variant{ value.GetBool() };
	};
	map[rttr::type::get<float>().get_id()]
		= [](rapidjson::Value& value)
	{
		return rttr::variant{ value.GetFloat() };
	};
	map[rttr::type::get<int>().get_id()]
		= [](rapidjson::Value& value)
	{
		return rttr::variant{ value.GetInt() };
	};
	map[rttr::type::get<size_t>().get_id()]
		= [](rapidjson::Value& value)
	{
		return rttr::variant{ value.GetUint64() };
	};
	map[rttr::type::get<std::string>().get_id()]
		= [](rapidjson::Value& value)
	{
		return rttr::variant{ std::string{value.GetString()} };
	};
	/*------------------------
	rttr types
	------------------------*/
	map[rttr::type::get<rttr::variant>().get_id()]
		= [](rapidjson::Value& value)
	{
		assert(value.IsObject());
		auto rttr_obj = value.GetObj();

		return rttr::variant{ oo::Anim::internal::LoadRTTRVariant(rttr_obj)};
	};
	map[rttr::type::get<rttr::type>().get_id()]
		= [](rapidjson::Value& value)
	{
		 std::string str = value.GetString();

		return rttr::variant{ rttr::type::get_by_name(str) };
	};
	map[rttr::type::get<rttr::property>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto obj = value.GetObj(); 
		auto var = oo::Anim::internal::loadDataFn_map.at(rttr::type::get<rttr::type>().get_id())(obj.FindMember("Type")->value).get_value<rttr::type>();
		auto prop = var.get_property(obj.FindMember("Property")->value.GetString());
		return rttr::variant{ prop };
	};
	/*------------------------
	complex types
	------------------------*/
	map[rttr::type::get<glm::vec3>().get_id()]
		= [](rapidjson::Value& value)
	{
		glm::vec3 vec{};
		auto vec_obj = value.GetObj();
		vec.x = vec_obj.FindMember("X")->value.GetFloat();
		vec.y = vec_obj.FindMember("Y")->value.GetFloat();
		vec.z = vec_obj.FindMember("Z")->value.GetFloat();

		return rttr::variant{ vec };
	};

	map[rttr::type::get<glm::quat>().get_id()]
		= [](rapidjson::Value& value)
	{
		glm::quat quaternion{};
		auto vec_obj = value.GetObj();
		quaternion.x = vec_obj.FindMember("X")->value.GetFloat();
		quaternion.y = vec_obj.FindMember("Y")->value.GetFloat();
		quaternion.z = vec_obj.FindMember("Z")->value.GetFloat();
		quaternion.w = vec_obj.FindMember("W")->value.GetFloat();

		return rttr::variant{ quaternion };
	};

	map[rttr::type::get<Condition::CompareType>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto enum_value = rttr::type::get <Condition::CompareType>().get_enumeration().name_to_value(value.GetString());
		return rttr::variant{ enum_value };
	};

	map[rttr::type::get<oo::Anim::P_TYPE>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto enum_value = rttr::type::get <oo::Anim::P_TYPE>().get_enumeration().name_to_value(value.GetString());
		return rttr::variant{ enum_value };
	};

	map[rttr::type::get<oo::Anim::Timeline::TYPE>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto enum_value = rttr::type::get <oo::Anim::Timeline::TYPE>().get_enumeration().name_to_value(value.GetString());
		return rttr::variant{ enum_value };
	};

	map[rttr::type::get<oo::Anim::Timeline::DATATYPE>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto enum_value = rttr::type::get <oo::Anim::Timeline::DATATYPE>().get_enumeration().name_to_value(value.GetString());
		return rttr::variant{ enum_value };
	};

	map[rttr::type::get<oo::ScriptValue::function_info>().get_id()]
		= [](rapidjson::Value& value)
	{
		oo::ScriptValue::function_info info{};
		auto obj = value.GetObj();
		info.classNamespace = obj.FindMember("classNamespace")->value.GetString();
		info.className = obj.FindMember("className")->value.GetString();
		info.functionName = obj.FindMember("functionName")->value.GetString();
		return rttr::variant{ info };
	};
	/*------------------------
	references
	------------------------*/
	map[rttr::type::get<GroupRef>().get_id()]
		= [](rapidjson::Value& value)
	{
		GroupRef ref;
		auto load_fn = rttr::type::get<GroupRef>().get_method(internal::load_method_name);
		load_fn.invoke({}, value, ref);
		return rttr::variant{ ref };
	};

	map[rttr::type::get<NodeRef>().get_id()]
		= [](rapidjson::Value& value)
	{
		NodeRef ref;
		auto load_fn = rttr::type::get<NodeRef>().get_method(internal::load_method_name);
		load_fn.invoke({}, value, ref);
		return rttr::variant{ ref };
	};

	map[rttr::type::get<LinkRef>().get_id()]
		= [](rapidjson::Value& value)
	{
		LinkRef ref;
		auto load_fn = rttr::type::get<LinkRef>().get_method(internal::load_method_name);
		load_fn.invoke({}, value, ref);
		return rttr::variant{ ref };
	};

	map[rttr::type::get<AnimRef>().get_id()]
		= [](rapidjson::Value& value)
	{
		AnimRef ref;
		auto load_fn = rttr::type::get<AnimRef>().get_method(internal::load_method_name);
		load_fn.invoke({}, value, ref);
		return rttr::variant{ ref };
	};
	return map;
}();


std::unordered_map<rttr::type::type_id, oo::Anim::internal::LoadFn*> oo::Anim::internal::loadRTTRVariantFn_map = []()
{
	std::unordered_map<rttr::type::type_id, oo::Anim::internal::LoadFn*> map;

	map[rttr::type::get<bool>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto val = value.GetBool();
		return rttr::variant{ val };
	};

	map[rttr::type::get<float>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto val = value.GetFloat();
		return rttr::variant{ val };
	};

	map[rttr::type::get<int>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto val = value.GetInt();
		return rttr::variant{ val };
	};

	map[rttr::type::get<size_t>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto val = static_cast<size_t>(value.GetUint64());
		return rttr::variant{ val };
	};

	map[rttr::type::get<std::string>().get_id()]
		= [](rapidjson::Value& value)
	{
		auto val = std::string{ value.GetString() };
		return rttr::variant{ val };
	};

	map[rttr::type::get<glm::vec3>().get_id()]
		= [](rapidjson::Value& value)
	{
		assert(oo::Anim::internal::loadDataFn_map.contains(rttr::type::get<glm::vec3>().get_id()));
		return rttr::variant{ oo::Anim::internal::loadDataFn_map.at(rttr::type::get<glm::vec3>().get_id())(value)};
	};

	map[rttr::type::get<glm::quat>().get_id()]
		= [](rapidjson::Value& value)
	{
		assert(oo::Anim::internal::loadDataFn_map.contains(rttr::type::get<glm::quat>().get_id()));
		return rttr::variant{ oo::Anim::internal::loadDataFn_map.at(rttr::type::get<glm::quat>().get_id())(value) };
	};

	return map;
}();

rttr::variant oo::Anim::internal::LoadRTTRVariant(rapidjson::Value& value)
{
	auto hash = static_cast<StringHash::size_type>(value.FindMember("Type Hash")->value.GetUint64());
	assert(oo::Anim::internal::hash_to_rttrType.contains(hash));
	auto& type = oo::Anim::internal::hash_to_rttrType.at(hash);
	auto& rttr_value = value.FindMember("Value")->value;
	assert(oo::Anim::internal::loadRTTRVariantFn_map.contains(type.get_id()));
	return oo::Anim::internal::loadRTTRVariantFn_map.at(type.get_id())(rttr_value);
};
namespace oo::Anim::internal
{
	
	/*uint GetAnimationIndex(std::string const& name)
	{
		assert(Animation::name_to_index.contains(name));
		return Animation::name_to_index[name];
	}
	uint GetAnimationIndex(size_t id)
	{
		assert(Animation::ID_to_index.contains(id));
		return Animation::ID_to_index[id];
	}*/

	NodeRef CreateNodeReference(Group& group, size_t id)
	{
		int index = 0;
		for (auto& [key, node] : group.nodes)
		{
			if (node.node_ID == id)
			{
				//NodeRef ref{
				//	.nodes{&group.nodes},
				//	//.index{index},
				//	.id{id}
				//};
				NodeRef ref{&group.nodes,id	};
				return ref;
			}
			++index;
		}
		assert(false);
		return {};
	}
	NodeRef CreateNodeReference(std::map<size_t, Node>& node_container, size_t id)
	{
		int index = 0;
		for (auto& [key, node] : node_container)
		{
			if (node.node_ID == id)
			{
				//NodeRef ref{
				//	.nodes{&node_container},
				//	//.index{index},
				//	.id{id}
				//};
				NodeRef ref{ &node_container,id };
				return ref;
			}
			++index;
		}
		assert(false);
		return {};
	}
	GroupRef CreateGroupReference(AnimationTree& tree, size_t id)
	{
		int index = 0;
		for (auto& [key, group] : tree.groups)
		{
			if (group.groupID == id)
			{
				//GroupRef ref{
				//	.groups{&tree.groups},
				//	//.index{index},
				//	.id{id}
				//};
				GroupRef ref{ &tree.groups,id };
				return ref;
			}
			++index;
		}
		assert(false);
		return {};
	}
	LinkRef CreateLinkReference(Group& group, size_t id)
	{
		int index = 0;
		for (auto& [key, link] : group.links)
		{
			if (link.linkID == id)
			{
				//LinkRef ref{
				//	.links{&group.links},
				//	//.index{index},
				//	.id{id}
				//};
				LinkRef ref{ &group.links, id };
				return ref;
			}
			++index;
		}

		assert(false);
		return {};
	}

	AnimRef CreateAnimationReference(size_t id)
	{
		AnimRef ref{ &(Animation::animation_storage), id };
		return ref;
	}

	TimelineRef CreateTimelineReference(AnimRef anim_ref, std::string const& timeline_name)
	{
		int index = 0;
		bool found = false;
		for (auto& timeline : anim_ref->timelines)
		{
			if (timeline.name == timeline_name)
			{
				found = true;
				break;
			}
			++index;
		}
		if (found)
			return TimelineRef{ .anim{anim_ref}, .index{index} };
		else
			return {};
	}

	Parameter::DataType ParameterDefaultValue(P_TYPE const type)
	{
		switch (type)
		{
		case P_TYPE::BOOL:
		case P_TYPE::TRIGGER:
			return false;
			break;
		case P_TYPE::INT:
			return 0;
			break;
		case P_TYPE::FLOAT:
			return 0.f;
			break;
		}

		assert(false);//unknown type?!?!
		return false;
	}

	Parameter::DataType ConditionDefaultValue(P_TYPE const type)
	{
		switch (type)
		{
		case P_TYPE::BOOL:
		case P_TYPE::TRIGGER:
			return true;
			break;
		case P_TYPE::INT:
			return 0;
			break;
		case P_TYPE::FLOAT:
			return 0.f;
			break;
		}

		assert(false); //unknown type?!?!
		return true;
	}


	bool TypeMatchesDataType(Parameter* parameter, Parameter::DataType const& value)
	{
		if (value.is_type<bool>())
			return parameter->type == P_TYPE::BOOL || parameter->type == P_TYPE::TRIGGER;

		if (value.is_type<int>())
			return parameter->type == P_TYPE::INT;

		if (value.is_type<float>())
			return parameter->type == P_TYPE::FLOAT;

		assert(false);
		return false;
	}

	bool ConditionSatisfied(Condition& condition, AnimationTracker& tracker)
	{
		//bool result = (condition->parameter->value == condition->value);

		////if its a trigger, comsume the trigger and set it back to false
		//if (condition->type == P_TYPE::TRIGGER && result)
		//	condition->parameter->SetWithoutChecking(false);
		assert(condition.compareFn);
		if (condition.compareFn)
			return condition.compareFn(condition.value, tracker.parameters[condition.parameterIndex].value);

		return false;
	}

	void AssignComparisonFunctionToCondition(Condition& condition)
	{
		assert(Condition::comparisonFn_map.contains(condition.type));

		if (Condition::comparisonFn_map.at(condition.type).contains(condition.comparison_type))
			condition.compareFn = Condition::comparisonFn_map.at(condition.type).at(condition.comparison_type);
		else
			condition.compareFn = nullptr;
	}

	//checks if currrent time has progressed past target time
	inline bool Withinbounds(float current, float target)
	{
		return target < current;
	}

	void TriggerEvent(UpdateProgressTrackerInfo& info, ScriptEvent& event)
	{
		event.script_function_info.Invoke(info.tracker_info.uuid);
	}

	KeyFrame::DataType GetInterpolatedValue(rttr::type rttr_type, KeyFrame::DataType prev, KeyFrame::DataType next, float percentage)
	{
		if (rttr_type == rttr::type::get< glm::vec3>())
		{
			return (1.f - percentage) * prev.get_value< glm::vec3 >() + (percentage * next.get_value< glm::vec3 >());
		}
		else if (rttr_type == rttr::type::get< glm::quat>())
		{
			return glm::slerp(prev.get_value< glm::quat >(), next.get_value< glm::quat >(), percentage);
			//return (1.f - percentage) * prev.get_value< glm::quat >() + (percentage * next.get_value< glm::quat >());
		}
		else if (rttr_type == rttr::type::get< bool>())
		{
			return percentage > 0.5f ? next : prev;
		}
		else return prev;
	}

	//void UpdateEvent(AnimationComponent& comp, AnimationTracker& tracker, ProgressTracker& progressTracker, float updatedTimer)
	void UpdateEvent(UpdateProgressTrackerInfo& info, float updatedTimer)
	{
		//it should be an event tracker
		assert(info.progressTracker.type == Timeline::TYPE::SCRIPT_EVENT);

		auto& timeline = info.tracker_info.tracker.currentNode->GetAnimation().events;
		//no events in timeline
		if (timeline.empty()) return;
		//already hit last so we return
		if (info.progressTracker.index >= (timeline.size() - 1ul))
			return;

		auto& nextEvent = *(timeline.begin() + info.progressTracker.index + 1ul);

		//if next event not within bounds
		if (Withinbounds(updatedTimer, nextEvent.time) == false) return;

		TriggerEvent(info, nextEvent);
		info.progressTracker.index++;

		//TODO: if went past more than 1 event, trigger those as well
	}

	//void UpdateProperty_Animation(AnimationComponent& comp, AnimationTracker& tracker, ProgressTracker& progressTracker, float updatedTimer)
	void UpdateProperty_Animation(UpdateProgressTrackerInfo& info, float updatedTimer)
	{
		//verify progress tracker correct type
		assert(info.progressTracker.type == Timeline::TYPE::PROPERTY);
		//it should be linked to a timeline!!
		assert(info.progressTracker.timeline != nullptr);


		auto& timeline = *(info.progressTracker.timeline);
		//verify correct timeline type
		assert(timeline.type == Timeline::TYPE::PROPERTY);
		//no keyframes so we return
		if (timeline.keyframes.empty()) return;

		//already hit last and animation not looping so we return
		if (info.progressTracker.index >= (timeline.keyframes.size() - 1ul) &&
			info.tracker_info.tracker.currentNode->GetAnimation().looping == false)
		{
			return;
		}

		////if looping, set the normalized time based on iterations
		//if (info.tracker_info.tracker.currentNode->GetAnimation().looping)
		//{
		//	if (info.tracker_info.tracker.timer > timeline.keyframes.back().time)
		//	{
		//		currentTimer -= timeline.keyframes.back().time;
		//		num_iterations += 1.f;
		//	}
		//	//set normalized timer
		//	info.tracker_info.tracker.normalized_timer = num_iterations + (currentTimer / timeline.keyframes.back().time);
		//}
		//else
		//{
		//	//set normalized timer
		//	info.tracker_info.tracker.normalized_timer = (updatedTimer / timeline.keyframes.back().time);
		//}


		//find the correct gameobject
		GameObject go{ info.tracker_info.entity,info.tracker_info.system.Get_Scene() };
		//traverse the hierarchy
		for (auto& index : timeline.children_index)
		{
			auto children = go.GetDirectChilds();
			go = children[index];
		}
		//if next keyframe within bounds increment index 
		auto& nextEvent = *(timeline.keyframes.begin() + info.progressTracker.index + 1ul);
		if (Withinbounds(updatedTimer, nextEvent.time))
		{
			info.progressTracker.index++;

			//went past last keyframe so we set data to last and return
			if (info.progressTracker.index >= (timeline.keyframes.size() - 1ul))
			{
				//get the instance
				auto ptr = info.tracker_info.system.Get_Ecs_World()->get_component(
					go.GetEntity(), info.progressTracker.timeline->component_hash);
				auto rttr_instance = hash_to_instance[info.progressTracker.timeline->component_hash](ptr);
				//set the value
				info.progressTracker.timeline->rttr_property.set_value(rttr_instance, timeline.keyframes.back().data);

				//if animation is looping, reset keyframe index
				if (info.tracker_info.tracker.currentNode->GetAnimation().looping)
				{
					info.progressTracker.index = 0;
				}
				return;
			}

		}

		/*--------------------------------
		interpolate animation accordingly
		--------------------------------*/
		auto& prevKeyframe = *(timeline.keyframes.begin() + info.progressTracker.index);
		auto& nextKeyframe = *(timeline.keyframes.begin() + info.progressTracker.index + 1u);
		auto prevTime = prevKeyframe.time;
		auto nextTime = nextKeyframe.time;

		//current progress in keyframe over total time in between keyframes
		float percentage = (updatedTimer - prevTime) / (nextTime - prevTime);

		KeyFrame::DataType interpolated_value = GetInterpolatedValue(
			info.progressTracker.timeline->rttr_type, prevKeyframe.data, nextKeyframe.data, percentage);

		/*--------------------------------
		set related game object's data
		--------------------------------*/
		//get a ptr to the component
		auto ptr = info.tracker_info.system.Get_Ecs_World()->get_component(
			go.GetEntity(), info.progressTracker.timeline->component_hash);

		//get the instance
		auto rttr_instance = hash_to_instance[info.progressTracker.timeline->component_hash](ptr);
		//set the value
		info.progressTracker.timeline->rttr_property.set_value(rttr_instance, interpolated_value);

		//SetComponentData(timeline, interpolated_value);
		//assert(false);
	}

	//void UpdateFBX_Animation(AnimationComponent& comp, AnimationTracker& tracker, ProgressTracker& progressTracker, float updatedTimer)
	void UpdateFBX_Animation(UpdateProgressTrackerInfo& info, float updatedTimer)
	{
		//assert(progressTracker.type == Timeline::TYPE::FBX_ANIM);
	}
	//go through all progress trackers and call their update function
	void UpdateTrackerKeyframeProgress(UpdateTrackerInfo& info, float updatedTimer)
	{
		for (auto& progressTracker : info.tracker.trackers)
		{
			//it should have an update function!!
			assert(progressTracker.updatefunction != nullptr);
			UpdateProgressTrackerInfo p_info{ info, progressTracker };
			//call the respective update function on this tracker
			progressTracker.updatefunction(p_info, updatedTimer);
		}
	}

	KeyFrame* GetCurrentKeyFrame(ProgressTracker& tracker)
	{
		return &(tracker.timeline->keyframes[tracker.index]);
	}

	void UpdateTrackerTransitionProgress(UpdateTrackerInfo& info, float updatedTimer)
	{
		auto& trans_info = info.tracker.transition_info;
		trans_info.transition_timer += info.dt;
		//if timer still not past offset, continue as normal
		if (trans_info.transition_timer < trans_info.transition_offset)
		{
			UpdateTrackerKeyframeProgress(info, updatedTimer);
			return;
		}
		//interpolate between the src and dst keyframes
		auto src_percentage = (trans_info.transition_timer - trans_info.transition_offset) / trans_info.transition_duration;
		auto dst_percentage = 1.f - src_percentage;
		(void)dst_percentage;
		//TODO: there is multiple keyframes, and we can only interpolate the same type ones
		/*for (auto& trackers : info.tracker.trackers)
		{

		}*/

		//auto kf = GetCurrentKeyFrame(info.tracker.trackers)

	}
	//void UpdateTrackerKeyframeProgress(AnimationComponent& component, AnimationTracker& tracker, float updatedTimer)
	//{
	//	for (auto& progressTracker : tracker.trackers)
	//	{
	//		//it should have an update function!!
	//		assert(progressTracker.updatefunction != nullptr);
	//		//call the respective update function on this tracker
	//		progressTracker.updatefunction(component, tracker, progressTracker, updatedTimer);
	//	}
	//}

	//set current node
	//copy the node's trackers
	//reset timer to 0.0f
	//void SetTrackerCurrentNode(AnimationTracker& tracker, Node& node)
	//{
	//	tracker.currentNode = &node;
	//	//tracker.trackers = node.trackers;
	//	tracker.timer = 0.0f;
	//}

	void AssignNodeToTracker(AnimationTracker& animTracker, NodeRef node)
	{
		//set current node
		animTracker.currentNode = node;
		//reset timers
		animTracker.timer = 0.f;
		animTracker.normalized_timer = 0.f;
		animTracker.global_timer = 0.f;
		animTracker.num_iterations = 0;
		//track all timelines in node's animations with trackers
		animTracker.trackers = node->trackers;
	}

	//copy animation tree's parameters to the tracker
	//set the starting node for the tracker and its respective data
	/*void InitializeTracker(IAnimationComponent& comp)
	{
		comp.tracker.parameters = comp.animTree->parameters;
		SetTrackerCurrentNode(comp.tracker, *(comp.animTree->groups.front().startNode));

	}*/

	//update a node's trackers to reflect its animation timelines
	void UpdateNodeTrackers(Node& node)
	{
		node.trackers.clear();
		//add script event tracker
		{
			auto progressTracker = ProgressTracker::Create(Timeline::TYPE::SCRIPT_EVENT);
		}

		//add timeline trackers
		for (auto& timeline : node.GetAnimation().timelines)
		{
			auto progressTracker = ProgressTracker::Create(timeline.type);
			//assign it the timeline
			progressTracker.timeline = &timeline;
			//assign it to the node
			node.trackers.emplace_back(std::move(progressTracker));
		}
	}
	//checks if a node is available for transition
	Link* CheckNodeTransitions(UpdateTrackerInfo& info)
	{
		for (auto& link : info.tracker.currentNode->outgoingLinks)
		{
			if (link->has_exit_time)
			{
				//if exit time not reached continue
				if (info.tracker.normalized_timer < link->exit_time)
					continue;
				//if no conditions return link
				if (link->conditions.empty())
					return &(*link);

				//Check conditions
				bool all_cleared = true;
				for (auto& condition : link->conditions)
				{
					if (condition.Satisfied(info.tracker) == false)
						all_cleared = false;
				}
				if (all_cleared)
					return &(*link);

				//continue if not all conditions met
				continue;
			}
			else //no exit time so just check conditions
			{
				//Check conditions
				bool all_cleared = true;
				for (auto& condition : link->conditions)
				{
					if (condition.Satisfied(info.tracker) == false)
						all_cleared = false;
				}
				if (all_cleared)
					return &(*link);

				continue;
			}
		}

		return nullptr;
	}

	void ActivateTransition(UpdateTrackerInfo& info, Link* link)
	{
		AssignNodeToTracker(info.tracker, link->dst);

		//TODO: transitions
		/*info.tracker.transition_info.in_transition = true;
		info.tracker.transition_info.link = link;
		info.tracker.transition_info.transition_timer = 0.f;
		info.tracker.transition_info.transition_timer = link->transition_offset;
		info.tracker.transition_info.transition_duration = link->transition_duration;

		info.tracker.transition_info.trackers = link->dst->trackers;*/

	}

	void UpdateTracker(UpdateTrackerInfo& info)
	{
		auto result = CheckNodeTransitions(info);
		if (result)
		{
			ActivateTransition(info, result);
		}
		float updatedTimer = info.tracker.timer + info.tracker.currentNode->speed * info.dt;
		//update tracker timer and global timer
		info.tracker.timer = updatedTimer;
		info.tracker.global_timer += info.tracker.currentNode->speed * info.dt;
		info.tracker.normalized_timer = updatedTimer / info.tracker.currentNode->GetAnimation().animation_length;

		//not in transition
		if (info.tracker.transition_info.in_transition == false)
		{
			//check if we passed a keyframe and update
			UpdateTrackerKeyframeProgress(info, updatedTimer);
		}
		else
		{
			//interpolate between src and dst nodes
			UpdateTrackerTransitionProgress(info, updatedTimer);
		}
		//update timer and iterations if animation is looping
		if (info.tracker.currentNode->GetAnimation().looping &&
			updatedTimer > info.tracker.currentNode->GetAnimation().animation_length)
		{
			info.tracker.timer = updatedTimer - info.tracker.currentNode->GetAnimation().animation_length;
			++info.tracker.num_iterations;
		}

	}

	void AssignAnimationTreeToComponent(IAnimationComponent& component)
	{
		component.animTree = &(AnimationTree::map[component.animTree_name]);
	}

	void AssignAnimationTreeToComponent(IAnimationComponent& component, std::string const& name)
	{
		//name should not be empty!!
		assert(name.empty() == false);
		//retrieve tree
		if (AnimationTree::map.contains(name) == false)
		{
			LOG_CORE_DEBUG_CRITICAL("cannot find animation tree {0}!!", name);
			assert(false);
			return;
		}
		component.animTree_name = name;
		AssignAnimationTreeToComponent(component);
	}

	Group* RetrieveGroupFromTree(AnimationTree& tree, std::string const& groupName)
	{
		for (auto& [key, group] : tree.groups)
		{
			if (group.name == groupName)
				return &group;
		}

		LOG_CORE_ERROR("could not find {0} group!!", groupName);
		assert(false);
		return nullptr;
	}

	Node* RetrieveNodeFromTree(AnimationTree& tree, std::string const& groupName, std::string const& name)
	{
		auto group = RetrieveGroupFromTree(tree, groupName);

		if (group == nullptr) {
			assert(false);
			return nullptr;
		}

		for (auto& [key, node] : group->nodes)
		{
			if (node.name == name)
				return &node;
		}

		LOG_CORE_ERROR("could not find {0} node!!", groupName);
		assert(false);
		return nullptr;
	}

	Node* RetrieveNodeFromGroup(Group& group, std::string const& name)
	{
		for (auto& [key, node] : group.nodes)
		{
			if (node.name == name)
				return &node;
		}

		LOG_CORE_ERROR("could not retrieve {0} node in {1} group!!", name, group.name);
		assert(false);
		return nullptr;
	}
	//same as RetrieveNodeFromGroup but without error messages and asserts
	Node* TryRetrieveNodeFromGroup(Group& group, std::string const& name)
	{
		for (auto& [key, node] : group.nodes)
		{
			if (node.name == name)
				return &node;
		}
		return nullptr;
	}

	Link* RetrieveLinkFromGroup(Group& group, std::string const& linkName)
	{
		for (auto& [key, link] : group.links)
		{
			if (link.name == linkName)
				return &link;
		}

		LOG_CORE_ERROR("could not find {0} link!!", linkName);
		assert(false);
		return nullptr;
	}

	Parameter* RetrieveParameterFromTree(AnimationTree& tree, std::string const& param_name)
	{
		for (auto& param : tree.parameters)
		{
			if (param.name == param_name)
				return &param;
		}
		return nullptr;
	}

	Timeline* RetrieveTimelineFromAnimation(Animation& animation, std::string const& timelineName)
	{
		for (auto& timeline : animation.timelines)
		{
			if (timeline.name == timelineName)
				return &timeline;
		}
		LOG_CORE_ERROR("could not find {0} timeline in animation!!", timelineName);
		assert(false);
		return nullptr;
	}

	Timeline* TryRetrieveTimelineFromAnimation(Animation& animation, std::string const& timelineName)
	{
		for (auto& timeline : animation.timelines)
		{
			if (timeline.name == timelineName)
				return &timeline;
		}
		return nullptr;
	}

	Parameter* RetrieveParameterFromComponent(IAnimationComponent& comp, std::string const& paramName)
	{
		for (auto& param : comp.tracker.parameters)
		{
			if (param.name == paramName)
			{
				return &param;
			}
		}
		return nullptr;
	}

	Parameter* RetrieveParameterFromComponent(IAnimationComponent& comp, size_t id)
	{
		for (auto& param : comp.tracker.parameters)
		{
			if (param.paramID == id)
			{
				return &param;
			}
		}
		return nullptr;
	}
	

	Parameter* RetrieveParameterFromComponentByIndex(IAnimationComponent& comp, uint index)
	{
		assert(index < comp.tracker.parameters.size());
		return &(comp.tracker.parameters[index]);
	}

	Animation* RetrieveAnimation(std::string const& anim_name)
	{
		assert(Animation::name_to_ID.contains(anim_name));
		auto id = Animation::name_to_ID[anim_name];
		assert(Animation::animation_storage.contains(id));
		return &(Animation::animation_storage[id]);

	}

	Node* AddNodeToGroup(Group& group, Anim::NodeInfo& info)
	{

		//if node already added to this group then just return it
		for (auto& [key, node] : group.nodes)
		{
			if (node.name == info.name)
			{
				LOG_CORE_WARN("{0} Animation Node already exists in group!!", info.name);
				return &node;
			}
		}

		//create the node and add it to this group
		Node node{ info };
		size_t key = node.node_ID;
		//UpdateNodeTrackers(node);
		auto [iter, result] = group.nodes.insert(std::make_pair(key, std::move(node)));
		assert(result == true); //insertion should occur, node should not be already existing!!
		return &group.nodes[key];

	}

	Group* AddGroupToTree(AnimationTree& tree, GroupInfo& info)
	{
		info.tree = &tree;
		Group new_group{ info };
		size_t key = new_group.groupID;
		auto [iter, result] = tree.groups.emplace(key, std::move(new_group));
		assert(result == true);
		auto& group = tree.groups[key];
		//create the starting node
		NodeInfo n_info{
			.name{ "Start Node" },
			.animation_name{ Animation::empty_animation_name },
			.speed{ 1.f },
			.position{0.f,0.f,0.f},
			.group{ internal::CreateGroupReference(tree,group.groupID)},
			.nodeID{internal::generateUID() }
		};
		auto node = Anim::internal::AddNodeToGroup(group, n_info);
		assert(node);
		group.startNode = internal::CreateNodeReference(group, n_info.nodeID);

		return &group;
	}



	//Node* AddNodeToGroup(AnimationTree& tree, std::string const& groupName, Anim::NodeInfo& info)
	//{
	//	Group* group = RetrieveGroupFromTree(tree, groupName);
	//	if (group == nullptr)
	//	{
	//		LOG_WARN("could not find {0} group to add {1} node!!", groupName, info.name);
	//		return nullptr;
	//	}
	//	//create the node and add it to this group
	//	Node node{*group, name };
	//	return AddNodeToGroup(*group, name, position);
	//}

	Timeline* AddTimelineToAnimation(Animation& animation, Anim::TimelineInfo const& info)
	{
		Timeline* existing_timeline = TryRetrieveTimelineFromAnimation(animation, info.timeline_name);

		//already exists so we just return it
		if (existing_timeline != nullptr)
			return existing_timeline;

		Anim::TimelineInfo createInfo = info;

		//detect gameobject hierarchy
		if (info.hierarchy_provided == false)
		{
			oo::UUID current = info.source_object.GetInstanceID();
			oo::UUID target = info.target_object.GetInstanceID();
			//if the target gameobject is not the root
			if (current != target)
			{
				//generate hierarchy map
				std::unordered_map<oo::UUID::value_type, std::vector<int>> uuid_hierarchy_map{};
				{
					std::function<void(oo::GameObject)> traverse_recursive = [&](oo::GameObject node)
					{
						if (node.HasChild() == false)
							return;

						int idx = 0;
						std::vector<int> children_index = uuid_hierarchy_map[node.GetInstanceID()];
						//add a new element
						children_index.emplace_back(idx);
						auto children = node.GetChildren();
						for (auto& child : children)
						{
							//set it to the correct index
							children_index.back() = idx;
							//assign it to the map
							uuid_hierarchy_map[child.GetInstanceID()] = children_index;
							//recurse
							traverse_recursive(child);
							//increment index
							++idx;
						}

					};

					std::vector<int> children_idx{};
					uuid_hierarchy_map[current] = children_idx;
					traverse_recursive(info.source_object);
				}

				createInfo.children_index = uuid_hierarchy_map[target];
			}

		}

		//create the new timeline
		Timeline timeline{ createInfo };
		return &(animation.timelines.emplace_back(std::move(timeline)));
	}

	//Timeline* AddTimelineToAnimation(Animation& animation, std::string const& timelineName, 
	//	Timeline::TYPE type, Timeline::DATATYPE datatype)
	//{
	//	Timeline* existing_timeline = TryRetrieveTimelineFromAnimation(animation, timelineName);

	//	//already exists so we just return it
	//	if (existing_timeline != nullptr)
	//		return existing_timeline;

	//	//create the new timeline
	//	Timeline timeline{ type ,datatype,timelineName };
	//	return &(animation.timelines.emplace_back(std::move(timeline)));
	//}

	bool KeyframeMatchesTimeline(Timeline& timeline, KeyFrame const& keyframe)
	{
		if (timeline.rttr_type == rttr::type::get<TransformComponent::quat>())
		{
			return timeline.rttr_type == keyframe.data.get_type() ||
				keyframe.data.get_type() == rttr::type::get<glm::quat>();
		}

		return timeline.rttr_type == keyframe.data.get_type();
		/*switch (timeline.datatype)
		{
		case Timeline::DATATYPE::VEC3:
			return keyframe.data.is_type<glm::vec3>();
			break;
		case Timeline::DATATYPE::QUAT:
			return keyframe.data.is_type<glm::quat>();
			break;
		case Timeline::DATATYPE::BOOL:
			return keyframe.data.is_type<bool>();
			break;
		default:
			return false;
			break;
		}*/
		/*switch (timeline.datatype)
		{
		case Timeline::DATATYPE::VEC3:
			return std::holds_alternative<glm::vec3>(keyframe.data);
			break;
		case Timeline::DATATYPE::QUAT:
			return std::holds_alternative<glm::quat>(keyframe.data);
			break;
		case Timeline::DATATYPE::BOOL:
			return std::holds_alternative<bool>(keyframe.data);
			break;
		default:
			return false;
			break;
		}*/
	}

	KeyFrame* AddKeyframeToTimeline(Timeline& timeline, KeyFrame const& keyframe)
	{
		assert(KeyframeMatchesTimeline(timeline, keyframe));
		//find index to insert, by finding first keyframe that is further
		size_t index = 0;
		for (auto& kf : timeline.keyframes)
		{
			if (kf.time > keyframe.time)
			{
				break;
			}
			++index;
		}
		//insert
		auto iterator = timeline.keyframes.begin() + index;
		return &(*(timeline.keyframes.emplace(iterator, keyframe)));
	}

	ScriptEvent* AddScriptEventToAnimation(Animation& animation, ScriptEvent const& scriptevent)
	{
		//check if valid script function here later

		//find index to insert, by finding first keyframe that is further
		size_t index = 0;
		for (auto& event : animation.events)
		{
			if (event.time > scriptevent.time)
			{
				break;
			}
			++index;
		}
		//insert
		auto iterator = animation.events.begin() + index;
		return &(*(animation.events.emplace(iterator, scriptevent)));
	}

	//node->GetAnimation(), timelineName, type, datatype, keyframe
	//adds a link from src to dst nodes and assumes src and dst nodes are valid
	Link* AddLinkBetweenNodes(Group& group, std::string const& src_name, std::string const& dst_name)
	{
		//links only exist with valid source and destination nodes
		auto* src_node = TryRetrieveNodeFromGroup(group, src_name);
		auto* dst_node = TryRetrieveNodeFromGroup(group, dst_name);

		//source and destination nodes should exist
		assert(src_node != nullptr && dst_node != nullptr);
		auto src_ref = CreateNodeReference(group, src_node->node_ID);
		auto dst_ref = CreateNodeReference(group, dst_node->node_ID);
		Link link{ src_ref , dst_ref };
		link.name = src_node->name + " -> " + dst_node->name;
		size_t key = link.linkID;
		//auto& createdLink = group.links.emplace_back(std::move(link));
		auto [iter, result] = group.links.emplace(key, std::move(link));
		assert(result); //link should be inserted!!
		auto& createdLink = group.links[key];

		src_node->outgoingLinks.emplace_back(CreateLinkReference(group, createdLink.linkID));

		return &createdLink;
	}

	Parameter* AddParameterToTree(AnimationTree& tree, Anim::ParameterInfo const& info)
	{
		Parameter param{ info };
		auto& parameter = tree.parameters.emplace_back(std::move(param));
		tree.paramIDtoIndexMap[parameter.paramID] = static_cast<uint>(tree.parameters.size() - 1ull);

		return &parameter;
	}

	Condition* AddConditionToLink(AnimationTree& tree, Link& link, ConditionInfo& info)
	{
		//verify there is a parameter available
		assert(info._param);
		//set the parameter's unique id
		info._paramID = info._param->paramID;

		Condition condition{ info };
		auto& createdCondition = link.conditions.emplace_back(std::move(condition));
		return &createdCondition;
	}

	Animation* AddAnimationToNode(Node& node, Animation& anim)
	{
		node.anim = CreateAnimationReference(anim.animation_ID);

		return &anim;
	}

	void LoadFBX(std::string const& filepath, Animation* anim)
	{
		//Assimp::Importer importer;

		//uint flags = 0;
		//flags |= aiProcess_Triangulate;
		//flags |= aiProcess_GenSmoothNormals;
		//flags |= aiProcess_ImproveCacheLocality;
		//flags |= aiProcess_CalcTangentSpace;

		////load the file
		//const aiScene* scene = importer.ReadFile(filepath, flags);
		//if (scene == nullptr)
		//{
		//	LOG_CORE_ERROR("could not load FBX animation from {0}!!", filepath);
		//	assert(false);
		//	return;
		//}

		//if (scene->HasAnimations() == false)
		//{
		//	LOG_CORE_ERROR("could not find any animations in FBX file: {0}!!", filepath);
		//	assert(false);
		//	return;
		//}

		//std::vector<aiAnimation*> fbx_anims;
		//fbx_anims.reserve(scene->mNumAnimations);
		//for (uint i = 0; i < scene->mNumAnimations; i++)
		//{
		//	fbx_anims.emplace_back(scene->mAnimations + i);
		//}



	}



	void InitialiseComponent(IAnimationComponent& comp)
	{
		assert(comp.animTree);
		//copy parameters
		comp.tracker.parameters = comp.animTree->parameters;
		//set current node to start node
		assert(comp.animTree->groups.begin()->second.startNode);
		AssignNodeToTracker(comp.tracker, comp.animTree->groups.begin()->second.startNode);
	}

	void BindConditionToParameter(AnimationTree& tree, Condition& condition)
	{
		//set param index//set param index
		assert(tree.paramIDtoIndexMap.contains(condition.paramID));
		condition.parameterIndex = tree.paramIDtoIndexMap[condition.paramID];

		//set condition comparison function
		internal::AssignComparisonFunctionToCondition(condition);
	}

	void BindConditionsToParameters(AnimationTree& tree)
	{
		//assign all the parameter's index to the tree's ID to index mapping
		uint index = 0;
		for (auto& param : tree.parameters)
		{
			tree.paramIDtoIndexMap[param.paramID] = index;
			++index;
		}
		//for all conditions
		for (auto& [group_id, group] : tree.groups)
		{
			for (auto& [link_id, link] : group.links)
			{
				for (auto& condition : link.conditions)
				{
					BindConditionToParameter(tree, condition);
					////set param index
					//assert(tree.paramIDtoIndexMap.contains(condition.paramID));
					//condition.parameterIndex = tree.paramIDtoIndexMap[condition.paramID];

					////set condition comparison function
					//internal::AssignComparisonFunctionToCondition(condition);
				}
			}
		}
	}

	void BindNodesToAnimations(AnimationTree& tree)
	{
		for (auto& [group_id, group] : tree.groups)
		{
			for (auto& [node_id, node] : group.nodes)
			{
				node.anim.Reload();
				UpdateNodeTrackers(node);
			}
		}
	}


	void CalculateAnimationLength(AnimationTree& tree)
	{
		//for all nodes
		for (auto& [group_id, group] : tree.groups)
		{
			for (auto& [node_id, node] : group.nodes)
			{
				auto& animation = node.GetAnimation();
				float longest_time{ 0.f };

				for (auto& timeline : animation.timelines)
				{
					if (timeline.keyframes.empty()) continue;

					if (longest_time < timeline.keyframes.back().time)
						longest_time = timeline.keyframes.back().time;
				}

				animation.animation_length = longest_time;
			}
		}
	}

	void ReloadReferences(AnimationTree& tree)
	{
		for (auto& [group_id, group] : tree.groups)
		{
			group.startNode.Reload();
			for (auto& [node_id, node] : group.nodes)
			{
				node.group.Reload();
				for (auto& link : node.outgoingLinks)
					link.Reload();
			}
			for (auto& [link_id, link] : group.links)
			{
				link.src.Reload();
				link.dst.Reload();
			}
		}
	}

	void ReAssignReferences(AnimationTree& tree)
	{
		for (auto& [group_id, group] : tree.groups)
		{
			group.startNode.nodes = &group.nodes;
			for (auto& [node_id, node] : group.nodes)
			{
				node.group.groups = &tree.groups;
				for (auto& link : node.outgoingLinks)
					link.links = &group.links;
				node.anim.anims = &(Animation::animation_storage);
			}

			for (auto& [link_id, link] : group.links)
			{
				link.src.nodes = &group.nodes;
				link.dst.nodes = &group.nodes;
			}
		}
	}

	void CalculateParameterIndexes(AnimationTree& tree)
	{
		uint index{ 0 };
		for (auto& param : tree.parameters)
		{
			tree.paramIDtoIndexMap.insert_or_assign(param.paramID, index);
			++index;
		}
	}

	Animation* AddAnimationToStorage(std::string const& name)
	{
		Animation anim{};
		anim.name = name;

		auto key = anim.animation_ID;
		Animation::name_to_ID[name] = key;
		if (Animation::animation_storage.contains(key))
		{
			LOG_INFO("Replaced {0} animation", name);
		}
		else
		{
			LOG_INFO("Added {0} animation", name);
		}

		auto result = Animation::animation_storage.insert_or_assign(key, std::move(anim));
		assert(result.second);
		if (result.second == false)
		{
			LOG_CORE_ERROR("Error adding {0} animation!!", name);
			return nullptr;
		}

		return &(Animation::animation_storage[key]);
	}


	uint GetParameterIndex(IAnimationComponent& comp, std::string const& paramName)
	{
		uint index = 0;
		for (auto& param : comp.tracker.parameters)
		{
			if (param.name == paramName)
			{
				return index;
			}
			++index;
		}
		assert(false);
		LOG_CRITICAL("GetParameterIndex cannot find parameter {0}!!!", paramName);
		return std::numeric_limits<uint>().max();
	}
}