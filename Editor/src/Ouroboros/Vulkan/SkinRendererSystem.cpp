#include "pch.h"
#include "SkinRendererSystem.h"
#include <Ouroboros/Vulkan/MeshRendererComponent.h>

namespace oo
{
	SkinMeshRendererSystem::SkinMeshRendererSystem(GraphicsWorld* graphicsWorld)
		: m_graphicsWorld{ graphicsWorld }
	{
	}
	void SkinMeshRendererSystem::Init()
	{
		m_world->SubscribeOnAddComponent<SkinMeshRendererSystem, SkinMeshRendererComponent>(
			this, &SkinMeshRendererSystem::OnMeshAssign);

	}
	void SkinMeshRendererSystem::Run(Ecs::ECSWorld* world)
	{
		
		static Ecs::Query skin_mesh_query = Ecs::make_query<SkinMeshRendererComponent, TransformComponent>();
		
		static Ecs::Query skin_bone_mesh_query = Ecs::make_query<SkinMeshBoneComponent, TransformComponent>();

		world->for_each(skin_mesh_query,
			[&](SkinMeshRendererComponent& m_comp, TransformComponent& transformComp)
			{
				//do nothing if transform did not change
				if (transformComp.HasChangedThisFrame == false) return;


				auto& gfx_Object = m_graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);

				if (gfx_Object.bones.size() != m_comp.num_bones)
					gfx_Object.bones.resize(m_comp.num_bones);

				gfx_Object.modelID = m_comp.meshResource;
				gfx_Object.bindlessGlobalTextureIndex_Albedo = m_comp.albedoID;
				gfx_Object.bindlessGlobalTextureIndex_Normal = m_comp.normalID;
				gfx_Object.submesh = m_comp.meshInfo.submeshBits;

				gfx_Object.localToWorld = transformComp.GetGlobalMatrix();
			});

		world->for_each(skin_bone_mesh_query,
			[&](SkinMeshBoneComponent& boneComp, TransformComponent& transformComp)
			{
				//do nothing if transform did not change
				if (transformComp.HasChangedThisFrame == false) return;
				
				auto& gfx_Object = m_graphicsWorld->GetObjectInstance(boneComp.graphicsWorld_ID);
				
				//set bone matrix to inverse bind pose * matrix
				gfx_Object.bones[boneComp.inverseBindPose_info.boneIdx] =  transformComp.GetGlobalMatrix() * boneComp.inverseBindPose_info.transform;
			});

	}

	void SkinMeshRendererSystem::AssignGraphicsWorldID_to_BoneComponents(Ecs::ECSWorld* world)
	{
		static Ecs::Query skin_mesh_query = Ecs::make_query<SkinMeshRendererComponent, TransformComponent>();
		static Ecs::Query skin_bone_mesh_query = Ecs::make_query<SkinMeshBoneComponent, TransformComponent>();


	}

	void SkinMeshRendererSystem::OnMeshAssign(Ecs::ComponentEvent<SkinMeshRendererComponent>* evnt)
	{
		assert(m_world != nullptr);

		auto& meshComp = evnt->component;
		auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);

		meshComp.graphicsWorld_ID = m_graphicsWorld->CreateObjectInstance();
		
		//update initial position
		auto& graphics_object = m_graphicsWorld->GetObjectInstance(meshComp.graphicsWorld_ID);
		graphics_object.localToWorld = transform_component.GetGlobalMatrix();
		graphics_object.flags = ObjectInstanceFlags::SKINNED;

		meshComp.gfx_Object = &graphics_object;
	}
}
