#include "pch.h"
#include "Component_ActionCommand.h"

//components
#include <Ouroboros/Audio/AudioListenerComponent.h>
#include <Ouroboros/Audio/AudioSourceComponent.h>
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/ECS/DeferredComponent.h>
#include <Ouroboros/ECS/DuplicatedComponent.h>
#include <Ouroboros/Transform/TransformComponent.h>
#include <Ouroboros/Prefab/PrefabComponent.h>

#include <Ouroboros/Physics/RigidbodyComponent.h>
#include <Ouroboros/Physics/ColliderComponents.h>

#include <Ouroboros/Scripting/ScriptComponent.h>
#include <Ouroboros/Scripting/ScriptSystem.h>
#include <Ouroboros/Scripting/ScriptManager.h>
#include <Ouroboros/Vulkan/MeshRendererComponent.h>
#include <Ouroboros/Vulkan/ParticleEmitterComponent.h>
#include <Ouroboros/Vulkan/SkinRendererComponent.h>
#include <Ouroboros/Vulkan/LightComponent.h>
#include <Ouroboros/Vulkan/CameraComponent.h>

#include <Ouroboros/ECS/GameObjectDebugComponent.h>
#include "ActionCommandHelper.h"
namespace oo
{
	void oo::ActionCommandHelper::Init()
	{
		AddCommandToMap<oo::GameObjectComponent>();
		AddCommandToMap<oo::TransformComponent>();
		AddCommandToMap<oo::DeferredComponent>();
		AddCommandToMap<oo::DuplicatedComponent>();
		AddCommandToMap<oo::RigidbodyComponent>();
		AddCommandToMap<oo::SphereColliderComponent>();
		AddCommandToMap<oo::BoxColliderComponent>();
		AddCommandToMap<oo::MeshColliderComponent>();
		AddCommandToMap<oo::GameObjectDebugComponent>();
		AddCommandToMap<oo::MeshRendererComponent>();
		AddCommandToMap<oo::ParticleEmitterComponent>();
		AddCommandToMap<oo::SkinMeshRendererComponent>();
		AddCommandToMap<oo::SkinMeshBoneComponent>();
		AddCommandToMap<oo::DeferredComponent>();
		AddCommandToMap<oo::LightComponent>();
		AddCommandToMap<oo::CameraComponent>();
		AddCommandToMap<oo::AudioListenerComponent>();
		AddCommandToMap<oo::AudioSourceComponent>();

	}
	ActionCommand* ActionCommandHelper::CreateActionCommand(PacketHeader& header, const std::string& data)
	{
		size_t offset = 0;
		std::string component_name = PacketUtilts::ParseCommandData(data, offset);
		std::string new_data(data.begin() + offset, data.end());
		auto iter = s_createActionCommand.find(component_name);
		if (iter == s_createActionCommand.end())
		{
			ASSERT_MSG(true, "if this happen you might need to append to the function above");
		}
		return iter->second(header, new_data);
	}

	ActionCommand* ActionCommandHelper::CreateActionCommand_Add(PacketHeader& header, const std::string& data)
	{
		size_t offset = 0;
		std::string component_name = PacketUtilts::ParseCommandData(data, offset);
		std::string new_data(data.begin() + offset, data.end());
		auto iter = s_createAddComponentCommand.find(component_name);
		if (iter == s_createAddComponentCommand.end())
		{
			ASSERT_MSG(true, "if this happen you might need to append to the function above");
		}
		return iter->second(header, new_data);
	}

	ActionCommand* ActionCommandHelper::CreateActionCommand_Remove(PacketHeader& header, const std::string& data)
	{
		size_t offset = 0;
		std::string component_name = PacketUtilts::ParseCommandData(data, offset);
		std::string new_data(data.begin() + offset, data.end());
		auto iter = s_createRemoveComponentCommand.find(component_name);
		if (iter == s_createRemoveComponentCommand.end())
		{
			ASSERT_MSG(true, "if this happen you might need to append to the function above");
		}
		return iter->second(header, new_data);
	}

}


