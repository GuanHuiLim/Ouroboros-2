#include "pch.h"
#include "SkinRendererSystem.h"
#include <Ouroboros/Vulkan/MeshRendererComponent.h>
#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
	SkinMeshRendererSystem::SkinMeshRendererSystem(GraphicsWorld* graphicsWorld, oo::Scene* _scene)
		: m_graphicsWorld{ graphicsWorld },
		scene{_scene}
	{
	}
	void SkinMeshRendererSystem::Init()
	{
		m_world->SubscribeOnAddComponent<SkinMeshRendererSystem, SkinMeshRendererComponent>(
			this, &SkinMeshRendererSystem::OnMeshAssign);


		m_world->SubscribeOnRemoveComponent<SkinMeshRendererSystem, SkinMeshRendererComponent>(
			this, &SkinMeshRendererSystem::OnMeshRemove);
	}

	void RecurseChildren_AssignparentTransform_to_BoneComponents(GameObject obj, glm::mat4 parentTransform)
	{
		auto children = obj.GetDirectChilds();
		if (children.empty()) return;

		//auto localTransform = obj.GetComponent<TransformComponent>().GetLocalMatrix();

		for (auto& child : children)
		{
			if (child.HasComponent<SkinMeshBoneComponent>() == false) continue;

			auto const transform = parentTransform * child.GetComponent<TransformComponent>().GetLocalMatrix();
			child.GetComponent<SkinMeshBoneComponent>().globalTransform = transform;

			RecurseChildren_AssignparentTransform_to_BoneComponents(child, transform);
		}
	}
	void SkinMeshRendererSystem::Run(Ecs::ECSWorld* world)
	{
		
		static Ecs::Query skin_mesh_query = Ecs::make_query<SkinMeshRendererComponent, TransformComponent>();
		
		static Ecs::Query skin_bone_mesh_query = Ecs::make_query<SkinMeshBoneComponent, TransformComponent>();
		
		world->for_each_entity(skin_mesh_query,
			[&](Ecs::EntityID entity)
			{
				oo::GameObject go{ entity,*scene };

				auto graphicsID = go.GetComponent<SkinMeshRendererComponent>().graphicsWorld_ID;

				auto parent = go.GetParent();
				auto children = parent.GetDirectChilds();
				auto uid = go.GetInstanceID();
				oo::GameObject rootbone{};
				for (auto& child : children)
				{
					if (child.GetInstanceID() == uid) continue;

					rootbone = child;
					break;
				}
				//auto rootbone_global_inverse = glm::affineInverse(rootbone.GetComponent<TransformComponent>().GetGlobalMatrix());
				RecurseChildren_AssignparentTransform_to_BoneComponents(rootbone, glm::identity<glm::mat4>());

			});


		world->for_each(skin_mesh_query,
			[&](SkinMeshRendererComponent& m_comp, TransformComponent& transformComp)
			{
				auto& gfx_Object = m_graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);
				gfx_Object.modelID = m_comp.meshResource;
				gfx_Object.bindlessGlobalTextureIndex_Albedo = m_comp.albedoID;
				gfx_Object.bindlessGlobalTextureIndex_Normal = m_comp.normalID;
				gfx_Object.submesh = m_comp.meshInfo.submeshBits;

				//do nothing if transform did not change
				if (transformComp.HasChangedThisFrame == false) return;

				if (gfx_Object.bones.size() != m_comp.num_bones)
					gfx_Object.bones.resize(m_comp.num_bones);


				gfx_Object.localToWorld = transformComp.GetGlobalMatrix();
			});

		world->for_each(skin_bone_mesh_query,
			[&](SkinMeshBoneComponent& boneComp, TransformComponent& transformComp)
			{
				//do nothing if transform did not change
				if (transformComp.HasChangedThisFrame == false) return;
				
				auto& gfx_Object = m_graphicsWorld->GetObjectInstance(boneComp.graphicsWorld_ID);
				
				//set bone matrix to inverse bind pose * matrix
				gfx_Object.bones[boneComp.inverseBindPose_info.boneIdx] = boneComp.globalTransform * boneComp.inverseBindPose_info.transform;
			});

	}

	void SkinMeshRendererSystem::PostLoadScene(oo::Scene& scene)
	{
		static Ecs::Query skin_mesh_query = Ecs::make_query<SkinMeshRendererComponent, TransformComponent>();

		//resize bones container beforehand
		scene.GetWorld().for_each(skin_mesh_query,
			[&](SkinMeshRendererComponent& m_comp, TransformComponent& transformComp)
			{
				auto& gfx_Object = m_graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);
				
				gfx_Object.modelID = m_comp.meshResource;
				gfx_Object.bindlessGlobalTextureIndex_Albedo = m_comp.albedoID;
				gfx_Object.bindlessGlobalTextureIndex_Normal = m_comp.normalID;
				gfx_Object.submesh = m_comp.meshInfo.submeshBits;

				if (gfx_Object.bones.size() != m_comp.num_bones)
					gfx_Object.bones.resize(m_comp.num_bones);
			});

		AssignGraphicsWorldID_to_BoneComponents(scene);
	}

	void RecurseChildren_AssignGraphicsWorldID_to_BoneComponents(GameObject obj, uint32_t graphicsID)
	{
		auto children = obj.GetChildren();
		if (children.empty()) return;

		for (auto& child : children)
		{
			if (child.HasComponent<SkinMeshBoneComponent>() == false) continue;

			child.GetComponent<SkinMeshBoneComponent>().graphicsWorld_ID = graphicsID;
		}
	}

	void SkinMeshRendererSystem::AssignGraphicsWorldID_to_BoneComponents(oo::Scene& scene)
	{
		static Ecs::Query skin_mesh_query = Ecs::make_query<SkinMeshRendererComponent, TransformComponent>();
		//static Ecs::Query skin_bone_mesh_query = Ecs::make_query<SkinMeshBoneComponent, TransformComponent>();

		scene.GetWorld().for_each_entity(skin_mesh_query,
			[&](Ecs::EntityID entity)
			{
				oo::GameObject go{ entity,scene };

				auto graphicsID = go.GetComponent<SkinMeshRendererComponent>().graphicsWorld_ID;

				auto parent = go.GetParent();
				auto children = parent.GetDirectChilds();
				auto name = go.Name();
				oo::GameObject the_one_with_bones{};
				for (auto& child : children)
				{
					if (child.Name() == name) continue;

					the_one_with_bones = child;
				}


				RecurseChildren_AssignGraphicsWorldID_to_BoneComponents(the_one_with_bones, graphicsID);
				/*auto children = go.GetChildren();

				for (auto& child : children)
				{
					if (child.HasComponent<SkinMeshBoneComponent>() == false) continue;
					child.GetComponent< SkinMeshBoneComponent>().graphicsWorld_ID = graphicsID;


				}*/

			});

	}

	void SkinMeshRendererSystem::OnMeshAssign(Ecs::ComponentEvent<SkinMeshRendererComponent>* evnt)
	{
		assert(m_world != nullptr);

		auto& meshComp = evnt->component;
		auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);
		auto& go_component = m_world->get_component<GameObjectComponent>(evnt->entityID);
		meshComp.graphicsWorld_ID = scene->CreateGraphicsInstance(go_component.Id);
		
		//update initial position
		auto& graphics_object = m_graphicsWorld->GetObjectInstance(meshComp.graphicsWorld_ID);
		graphics_object.localToWorld = transform_component.GetGlobalMatrix();
		graphics_object.flags = ObjectInstanceFlags::SKINNED | ObjectInstanceFlags::RENDER_ENABLED;

		meshComp.gfx_Object = &graphics_object;
	}
	void SkinMeshRendererSystem::OnMeshRemove(Ecs::ComponentEvent<SkinMeshRendererComponent>* evnt)
	{
		auto& comp = evnt->component; 
		scene->DestroyGraphicsInstance(comp.graphicsWorld_ID);
		// remove graphics id to uuid of gameobject
		//m_graphicsIdToUUID.erase(comp.GraphicsWorldID);
	}
}
