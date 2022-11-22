/************************************************************************************//*!
\file           AnimationTimeline.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A collection of keyframes that manipulate a single game object's component's value

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"
#include "AnimationTimeline.h"
#include "AnimationKeyFrame.h"
#include "AnimationInternal.h"

#include <rttr/registration>

namespace oo::Anim::internal
{
	void SerializeTimeline(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Timeline& timeline)
	{
		writer.StartObject();
		{
			
			//properties
			{
				rttr::instance obj{ timeline };
				auto properties = rttr::type::get<Timeline>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					auto temp_type = prop.get_type();
					rttr::variant val{ prop.get_value(obj) };
					auto value_type = val.get_type();
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}		
			//children_index
			{
				writer.Key("children_index", static_cast<rapidjson::SizeType>(std::string("children_index").size()));
				writer.StartArray();
				{
					for (auto& index : timeline.children_index)
						writer.Int(index);
				}
				writer.EndArray();
			}
			//keyframes
			writer.Key("keyframes", static_cast<rapidjson::SizeType>(std::string("keyframes").size()));
			writer.StartArray();
			{
				auto serialize_fn = rttr::type::get<KeyFrame>().get_method(internal::serialize_method_name);
				for (auto& kf : timeline.keyframes)
				{
					auto result = serialize_fn.invoke({}, writer, kf);
					assert(result.is_valid());
				}
			}
			writer.EndArray();
		}
		writer.EndObject();
	}

	void LoadTimeline(rapidjson::GenericObject<false, rapidjson::Value>& object, Timeline& timeline)
	{
		rttr::instance obj{ timeline };
		//properties
		{
			auto properties = rttr::type::get<Timeline>().get_properties();
			for (auto& prop : properties)
			{
				auto& value = object.FindMember(prop.get_name().data())->value;

				assert(internal::loadDataFn_map.contains(prop.get_type().get_id()));
				rttr::variant val{ internal::loadDataFn_map.at(prop.get_type().get_id())(value) };
				auto tmp_type = val.get_type();
				auto result = prop.set_value(obj, val);
				assert(result);
			}
		}
		//children_index
		{
			auto children_index = object.FindMember("children_index")->value.GetArray();
			auto load_fn = rttr::type::get<LinkRef>().get_method(internal::load_method_name);
			assert(load_fn);
			for (auto& child : children_index)
			{
				timeline.children_index.emplace_back(std::move(child.GetInt()));
			}
		}
		//children_index
		{
			auto keyframes = object.FindMember("keyframes")->value.GetArray();
			auto load_fn = rttr::type::get<KeyFrame>().get_method(internal::load_method_name);
			assert(load_fn);
			for (auto& kf : keyframes)
			{
				KeyFrame new_kf{};
				auto result = load_fn.invoke({}, kf.GetObj(), new_kf);
				assert(result.is_valid());
				timeline.keyframes.emplace_back(std::move(new_kf));
			}
		}
	}
}

namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<Timeline>("Animation Timeline")
			.enumeration<Timeline::TYPE>("TYPE")
			(
				value("PROPERTY", Timeline::TYPE::PROPERTY),
				value("FBX_ANIM", Timeline::TYPE::FBX_ANIM),
				value("SCRIPT_EVENT", Timeline::TYPE::SCRIPT_EVENT)
			)
			.enumeration<Timeline::DATATYPE>("DATATYPE")
			(
				value("VEC3", Timeline::DATATYPE::VEC3),
				value("QUAT", Timeline::DATATYPE::QUAT),
				value("BOOL", Timeline::DATATYPE::BOOL)
			)
			
			.property("name", &Timeline::name)
			.property("type", &Timeline::type)
			.property("datatype", &Timeline::datatype)
			.property("rttr_type", &Timeline::rttr_type)
			.property("rttr_property", &Timeline::rttr_property)
			.property("component_hash", &Timeline::component_hash)
			.method(internal::serialize_method_name, &internal::SerializeTimeline)
			.method(internal::load_method_name, &internal::LoadTimeline)
			;
	}

	Timeline::Timeline(TimelineInfo const& info)
	: type{ info.type }
	, rttr_property{ info.rttr_property }
	, rttr_type{ info.rttr_property.get_type() }
	, name{ info.timeline_name }
	, component_hash{ info.component_hash }
	, children_index{ info.children_index }
	{
		get_componentFn = Ecs::ECSWorld::get_component_Fn(component_hash);

		//verify able to retrieve the component info
		assert(get_componentFn != nullptr);
	}
	/*-------------------------------
	ProgressTracker
	-------------------------------*/
	ProgressTracker::ProgressTracker(const Timeline::TYPE _type) :
	type{ _type }
	{

	}
	ProgressTracker ProgressTracker::Create(Timeline::TYPE type)
	{
		ProgressTracker tracker{ type };
		switch (type)
		{
		case Timeline::TYPE::PROPERTY:
			tracker.updatefunction = &internal::UpdateProperty_Animation;
			break;
		case Timeline::TYPE::FBX_ANIM:
			tracker.updatefunction = &internal::UpdateFBX_Animation;
			break;
		case Timeline::TYPE::SCRIPT_EVENT:
			assert(false);//it shouldnt hit this
			//tracker.updatefunction = &internal::UpdateEvent;
			break;
		default:
			break;
		}

		return tracker;
	}
}