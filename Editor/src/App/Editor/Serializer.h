#pragma once
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Scene/Scene.h>
#include <rttr/variant.h>
#include <rttr/property.h>
#include <rapidjson/document.h>

#include <unordered_map>
#include <functional>
#include <string>
#include "App/Editor/Utility/UI_RTTRType.h"
class Serializer
{
public:
	Serializer();
	~Serializer();
	static void Init();
	static void SaveScene(oo::Scene& scene);
	static void SaveObject(oo::GameObject& go, rapidjson::Value & val);

	template <typename Component>
	static void SaveComponent(oo::GameObject& go, rapidjson::Value& val);

private:
	//the function requires the user to insert the variant into the value manually
	inline static std::unordered_map < UI_RTTRType::UItypes, std::function<void(rapidjson::Value&, rttr::variant,rttr::property)>> save_commands;
	inline static rapidjson::Document doc;
};

template<typename Component>
inline void Serializer::SaveComponent(oo::GameObject& go, rapidjson::Value& val)
{
	if (go.HasComponent<Component>() == false)
		return;
	rapidjson::Value v(rapidjson::kObjectType);
	v.SetObject();
	Component& component = go.GetComponent<Component>();
	rttr::type type = component.get_type();
	for (auto& prop : type.get_properties())
	{
		if (prop.is_readonly())
			continue;

		auto iter = UI_RTTRType::types.find(prop.get_type().get_id());
		if (iter == UI_RTTRType::types.end())
			continue;//not supported
		auto sf = save_commands.find(iter->second);
		if (sf == save_commands.end())
			continue;//don't have this save function
		sf->second(v,prop.get_value(component),prop);
	}
	std::string temp = type.get_name().data();
	rapidjson::Value name;
	name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()),doc.GetAllocator());
	val.AddMember(name , v, doc.GetAllocator());
}
