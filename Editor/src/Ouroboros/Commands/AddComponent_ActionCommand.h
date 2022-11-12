#pragma once
#include "ActionCommand.h"

#include "Ouroboros/ECS/GameObject.h"
#include <SceneManagement/include/SceneManager.h>
#include "App/Editor/Utility/ImGuiManager.h"
#include "App/Editor/Networking/PacketUtils.h"

namespace oo
{
	/**
	 * Need to check for has component before adding this command
	 */
	template <typename Component>
	class AddComponent_ActionCommand :public ActionCommand
	{
	public:
		AddComponent_ActionCommand(oo::GameObject& go);
		~AddComponent_ActionCommand();
		void Undo() override;
		void Redo() override;
		std::string GetData();
		AddComponent_ActionCommand(PacketHeader& header, const std::string& data);
	private:
		oo::UUID id;
	};
	template<typename Component>
	inline AddComponent_ActionCommand<Component>::AddComponent_ActionCommand(oo::GameObject & go)
	{
		id = go.GetInstanceID();
		message = "added component : ";
		message += rttr::type::get<Component>().get_name().data();
		PacketUtilts::BroadCastCommand(CommandPacketType::AddComponentObject, GetData());
	}
	template<typename Component>
	inline AddComponent_ActionCommand<Component>::~AddComponent_ActionCommand()
	{
		message.clear();
	}
	template<typename Component>
	inline void AddComponent_ActionCommand<Component>::Undo()
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto gameobject = scene->FindWithInstanceID(id);
		if (gameobject == nullptr)
			return;
		gameobject->RemoveComponent<Component>();
	}
	template<typename Component>
	inline void AddComponent_ActionCommand<Component>::Redo()
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto gameobject = scene->FindWithInstanceID(id);
		if (gameobject == nullptr)
			return;
		gameobject->AddComponent<Component>();
	}
	template<typename Component>
	inline std::string AddComponent_ActionCommand<Component>::GetData()
	{
		rttr::type t = rttr::type::get<Component>();
		std::string data = "";
		data += t.get_name().data();//this will be trimed out before reaching the Constructor
		data += PacketUtilts::SEPERATOR;
		data += std::to_string(id.GetUUID());
		return data;
	}
	template<typename Component>
	inline AddComponent_ActionCommand<Component>::AddComponent_ActionCommand(PacketHeader& header, const std::string& data)
	{
		id = std::stoull(data);
		message = header.name;
		message += " added component";
		Redo();
	}
	//remove component
	template <typename Component>
	class RemoveComponent_ActionCommand :public ActionCommand
	{
	public:
		RemoveComponent_ActionCommand(oo::GameObject& go);
		~RemoveComponent_ActionCommand();
		void Undo() override;
		void Redo() override;
		std::string GetData();
		RemoveComponent_ActionCommand(PacketHeader& header, const std::string& data);
	private:
		oo::UUID id;
	};
	template<typename Component>
	inline RemoveComponent_ActionCommand<Component>::RemoveComponent_ActionCommand(oo::GameObject& go)
	{
		id = go.GetInstanceID();
		message = "Removed component : ";
		message += rttr::type::get<Component>().get_name().data();
		PacketUtilts::BroadCastCommand(CommandPacketType::RemoveComponentObject, GetData());
	}
	template<typename Component>
	inline RemoveComponent_ActionCommand<Component>::~RemoveComponent_ActionCommand()
	{
		message.clear();
	}
	template<typename Component>
	inline void RemoveComponent_ActionCommand<Component>::Undo()
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto gameobject = scene->FindWithInstanceID(id);
		if (gameobject == nullptr)
			return;
		gameobject->AddComponent<Component>();
	}
	template<typename Component>
	inline void RemoveComponent_ActionCommand<Component>::Redo()
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto gameobject = scene->FindWithInstanceID(id);
		if (gameobject == nullptr)
			return;
		gameobject->RemoveComponent<Component>();
	}
	template<typename Component>
	inline std::string RemoveComponent_ActionCommand<Component>::GetData()
	{
		rttr::type t = rttr::type::get<Component>();
		std::string data = "";
		data += t.get_name().data();//this will be trimed out before reaching the Constructor
		data += PacketUtilts::SEPERATOR;
		data += std::to_string(id.GetUUID());
		return data;
	}
	template<typename Component>
	inline RemoveComponent_ActionCommand<Component>::RemoveComponent_ActionCommand(PacketHeader& header, const std::string& data)
	{
		id = std::stoull(data);
		message = header.name;
		message += " removed component";
		Redo();
	}
}

