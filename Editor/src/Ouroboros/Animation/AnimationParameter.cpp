/************************************************************************************//*!
\file           AnimationParameter.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A variable referenced by conditions in animation links between nodes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AnimationParameter.h"
#include "AnimationInternal.h"

#include <rttr/registration>
namespace oo::Anim::internal
{
	void SerializeParameter(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Parameter& param)
	{
		writer.StartObject();
		{
			rttr::instance obj{ param };
			//properties
			{
				auto properties = rttr::type::get<Parameter>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					auto temp = prop.get_type().get_name();
					auto temp2 = val.get_type().get_name();
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
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
		registration::class_<Parameter>("Animation Parameter")
			.property("type", &Parameter::type)
			.property("value", &Parameter::value)
			.property("paramID", &Parameter::paramID)
			.property("name", &Parameter::name)
			.method(internal::serialize_method_name, &internal::SerializeParameter)
			;
	}
	Parameter::Parameter(ParameterInfo const& info) :
	name{ info.name }
	, type{ info.type }
	, paramID{ internal::generateUID() }
	{

		if (info.value.is_valid() == false)
			value = internal::ParameterDefaultValue(type);
		else
		{
			if (internal::TypeMatchesDataType(this, info.value) == false)
			{
				LOG_CORE_ERROR("Invalid {0} Parameter created!!! Value type different from parameter type!!");
				value = internal::ParameterDefaultValue(type);
				return;
			}

			value = info.value;
		}
	}
	void Parameter::Set(DataType const& _value)
	{
		//value to set should be same type!!
		assert(_value.get_type() != value.get_type());
		value = _value;
	}

}