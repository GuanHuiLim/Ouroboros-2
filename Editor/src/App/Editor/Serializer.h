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
#include "App/Editor/UI/Tools/WarningMessage.h"
class Serializer
{
public:
	Serializer();
	~Serializer();
	static void InitEvents();
	/*********************************************************************************//*!
	\brief     This Function should be called before any usuage of serializer
	*//**********************************************************************************/
	static void Init();
	/*********************************************************************************//*!
	\brief      Save the current Scene
	 
	\param      scene ==> read through the scene file
	
	*//**********************************************************************************/
	static void SaveScene(oo::Scene& scene);
	/*********************************************************************************//*!
	\brief      Load the Scene base on the filepath suggested by scenefile
	 
	\param      scene ==> will populate this scene
	
	*//**********************************************************************************/
	static void LoadScene(oo::Scene& scene);

	static void SavePrefab(std::shared_ptr<oo::GameObject> go);
	static void LoadPrefab(std::shared_ptr<oo::GameObject> go);
private:
	//saving
	static void SaveObject(oo::GameObject& go, rapidjson::Value & val);
	template <typename Component>
	static void SaveComponent(oo::GameObject& go, rapidjson::Value& val);
	//loading
	static void LoadObject(oo::GameObject& go, rapidjson::Value::MemberIterator& iter, rapidjson::Value::MemberIterator& end);
	template <typename Component>
	static void LoadComponent(oo::GameObject& go, rapidjson::Value&& val);
protected://rpj wrappers
	static void ResetDocument() noexcept;
protected://serialzation helpers
	template <typename Component>
	static void AddLoadComponent() noexcept;
private:
	//the function requires the user to insert the variant into the value manually
	//saving
	inline static std::unordered_map < UI_RTTRType::UItypes, std::function<void(rapidjson::Value&, rttr::variant,rttr::property)>> save_commands;
	//loading
	inline static std::unordered_map < rttr::type::type_id, std::function<void(oo::GameObject&,rapidjson::Value&&)>> load_components;
	inline static std::unordered_map < UI_RTTRType::UItypes, std::function<void(rttr::variant& , rapidjson::Value&&)>> load_commands;
	inline static rapidjson::Document doc;
	inline static constexpr int rapidjson_precision = 4;
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

template<typename Component>
inline void Serializer::LoadComponent(oo::GameObject& go, rapidjson::Value&& val)
{
	if (go.HasComponent<Component>() == false)
	{
		std::string msg = "GameObject does not contain this component: ";
		msg += rttr::type::get<Component>().get_name().data();
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, msg);
	}
	Component& component = go.GetComponent<Component>();
	for (auto iter = val.MemberBegin(); iter != val.MemberEnd(); ++iter)
	{
		rttr::type t = component.get_type();
		rttr::property prop = t.get_property(iter->name.GetString());
		if (prop.is_valid() == false)
			continue;
		
		auto types_UI = UI_RTTRType::types.find(prop.get_type().get_id());
		if (types_UI == UI_RTTRType::types.end())
			continue;//not supported
		auto command = load_commands.find(types_UI->second);
		if (command == load_commands.end())
			continue;//don't have this save function
		rttr::variant v;
		command->second(v, std::move(iter->value));
		prop.set_value(component, v);
	}
}

template<typename Component>
inline void Serializer::AddLoadComponent() noexcept
{
	load_components.emplace(rttr::type::get<Component>().get_id(),
		[](oo::GameObject& go, rapidjson::Value&& v) 
		{
			LoadComponent<Component>(go, std::move(v));
		});
}
