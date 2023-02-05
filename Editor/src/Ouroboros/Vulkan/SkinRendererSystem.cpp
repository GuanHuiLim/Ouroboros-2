#include "pch.h"
#include "SkinRendererSystem.h"
#include <Ouroboros/Vulkan/MeshRendererComponent.h>
#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "Project.h"

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

	void RecurseChildren_AssignparentTransform_to_BoneComponents(GameObject obj, glm::mat4 parentTransform, UUID uid)
	{
		auto children = obj.GetDirectChilds();
		if (children.empty()) return;
		//auto localTransform = obj.GetComponent<TransformComponent>().GetLocalMatrix();

		for (auto& child : children)
		{
			if (child.HasComponent<SkinMeshBoneComponent>() == false) continue;
			auto& bonecomp = child.GetComponent<SkinMeshBoneComponent>();
			if (bonecomp.root_bone_object != uid) continue;

			auto const transform = parentTransform * child.GetComponent<TransformComponent>().GetLocalMatrix();
			bonecomp.globalTransform = transform;

			RecurseChildren_AssignparentTransform_to_BoneComponents(child, transform, uid);
		}
	}

	void RecurseChildren_AssignGraphicsWorldID_UID_to_DuplicatedBoneComponents(GameObject obj, uint32_t graphicsID, UUID uid)
	{
		auto children = obj.GetChildren();
		if (children.empty()) return;

		for (auto& child : children)
		{
			if (child.HasComponent<SkinMeshBoneComponent>() == false) continue;

			auto& bonecomp = child.GetComponent<SkinMeshBoneComponent>();

			bonecomp.graphicsWorld_ID = graphicsID;
			bonecomp.root_bone_object = uid;
		}
	}

	void SkinMeshRendererSystem::Run(Ecs::ECSWorld* world)
	{
		
		static Ecs::Query skin_mesh_query = 
			Ecs::make_query<SkinMeshRendererComponent, TransformComponent>();
		static Ecs::Query duplicated_query = 
			Ecs::make_raw_query<SkinMeshRendererComponent, TransformComponent, GameObjectComponent, DuplicatedComponent>();
		static Ecs::Query skin_bone_mesh_query = 
			Ecs::make_query<SkinMeshBoneComponent, TransformComponent>();
		
		TRACY_PROFILE_SCOPE_NC(Skin_Mesh_Renderer_Update, 0x00E0E3);
		
		//settle duplicated objects
		world->for_each_entity_and_component(duplicated_query,
			[&](Ecs::EntityID entity,SkinMeshRendererComponent& renderComp, TransformComponent& transformComp, GameObjectComponent& goComp, DuplicatedComponent& dupComp)
			{
				Initialize(renderComp, transformComp, goComp);

				auto graphicsID = renderComp.graphicsWorld_ID;

				GameObject go{ entity,*scene };
				auto parent = go.GetParent();
				auto siblings = parent.GetDirectChilds();
				auto uid = go.GetInstanceID();
				oo::GameObject rootbone{};
				for (auto& child : siblings)
				{
					if (child.GetInstanceID() == uid)
					{
						continue;
					}
					rootbone = child;
					break;
				}

				RecurseChildren_AssignGraphicsWorldID_UID_to_DuplicatedBoneComponents(
					rootbone, graphicsID, uid);

			});

		//calculate transform
		world->for_each_entity_and_component(skin_mesh_query,
			[&](Ecs::EntityID entity, SkinMeshRendererComponent& m_comp, TransformComponent& transformComp)
			{
				{	//calculate inverse matrix for all root bones
					oo::GameObject go{ entity,*scene };

					//auto graphicsID = go.GetComponent<SkinMeshRendererComponent>().graphicsWorld_ID;

					auto parent = go.GetParent();
					auto children = parent.GetDirectChilds();
					auto uid = go.GetInstanceID();
					oo::GameObject rootbone{};
					for (auto& child : children)
					{
						if (child.GetInstanceID() == uid)
						{
							continue;
						}


						rootbone = child;
						break;
					}
					auto tmp = rootbone.Transform().GetGlobalMatrix();
					auto name = rootbone.Name();
					root_bone_inverse_map[uid] = glm::affineInverse(
						rootbone.Transform().GetGlobalMatrix());
					//auto rootbone_global_inverse = glm::affineInverse(rootbone.GetComponent<TransformComponent>().GetGlobalMatrix());
					//RecurseChildren_AssignparentTransform_to_BoneComponents(rootbone, glm::identity<glm::mat4>(), uid);
				}
				{	//assign graphics object properties in case it changed
					auto& gfx_Object = m_graphicsWorld->GetObjectInstance(
						m_comp.graphicsWorld_ID);
					gfx_Object.modelID = m_comp.meshResource;
					gfx_Object.bindlessGlobalTextureIndex_Albedo = m_comp.albedoID;
					gfx_Object.bindlessGlobalTextureIndex_Normal = m_comp.normalID;
					gfx_Object.submesh = m_comp.meshInfo.submeshBits;
					gfx_Object.SetShadowCaster(m_comp.CastShadows);
					gfx_Object.SetShadowReciever(m_comp.ReceiveShadows);
					//do nothing if transform did not change
					if (transformComp.HasChangedThisFrame == false) return;

					if (gfx_Object.bones.size() != m_comp.num_bones)
						gfx_Object.bones.resize(m_comp.num_bones);


					gfx_Object.localToWorld = transformComp.GetGlobalMatrix();

				}
			});


		//world->for_each(skin_mesh_query,
		//	[&](SkinMeshRendererComponent& m_comp, TransformComponent& transformComp)
		//	{
		//		auto& gfx_Object = m_graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);
		//		gfx_Object.modelID = m_comp.meshResource;
		//		gfx_Object.bindlessGlobalTextureIndex_Albedo = m_comp.albedoID;
		//		gfx_Object.bindlessGlobalTextureIndex_Normal = m_comp.normalID;
		//		gfx_Object.submesh = m_comp.meshInfo.submeshBits;
		//		gfx_Object.SetShadowCaster(m_comp.CastShadows);
		//		gfx_Object.SetShadowReciever(m_comp.ReceiveShadows);
		//		//do nothing if transform did not change
		//		if (transformComp.HasChangedThisFrame == false) return;

		//		if (gfx_Object.bones.size() != m_comp.num_bones)
		//			gfx_Object.bones.resize(m_comp.num_bones);


		//		gfx_Object.localToWorld = transformComp.GetGlobalMatrix();
		//	});

		//send data to graphics side
		world->for_each(skin_bone_mesh_query,
			[&](SkinMeshBoneComponent& boneComp, TransformComponent& transformComp)
			{
				//update the bone's transform
				boneComp.bone_transform = 
					root_bone_inverse_map[boneComp.root_bone_object] * transformComp.GetGlobalMatrix();
				//do nothing if transform did not change
				if (transformComp.HasChangedThisFrame == false) return;
				
				auto& gfx_Object = m_graphicsWorld->GetObjectInstance(boneComp.graphicsWorld_ID);
				
				//set bone matrix to inverse bind pose * matrix
				//gfx_Object.bones[boneComp.inverseBindPose_info.boneIdx] = boneComp.globalTransform * boneComp.inverseBindPose_info.transform;
				gfx_Object.bones[boneComp.inverseBindPose_info.boneIdx] 
					= boneComp.bone_transform * boneComp.inverseBindPose_info.transform;
			});

		TRACY_PROFILE_SCOPE_END();
	}

	ModelFileResource* FindModel_via_modelID(std::vector<Asset>& models, uint32_t id)
	{
		for (auto& model : models)
		{
			auto ptr = model.GetData<std::shared_ptr<ModelFileResource>>();
			if (ptr->meshResource == id)
				return ptr.get();
		}
		return nullptr;
	}

	/*void AssignGraphicsWorldID_to_Bones_Recursively(oo::GameObject obj, ModelFileResource* model, oGFX::BoneNode* curr)
	{
		uint index = 0;
		for (auto bone : curr->mChildren)
		{
			 
		}
	}*/

	void SkinMeshRendererSystem::PostLoadScene()
	{
		static Ecs::Query skin_mesh_query = Ecs::make_raw_query<SkinMeshRendererComponent, TransformComponent>();
		auto models = Project::GetAssetManager()->GetAssetsByType(AssetInfo::Type::Model);


		//resize bones container beforehand
		scene->GetWorld().for_each_entity_and_component(skin_mesh_query,
			[&](Ecs::EntityID entity,SkinMeshRendererComponent& m_comp, TransformComponent& transformComp)
			{
				auto& gfx_Object = m_graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);
				
				gfx_Object.modelID = m_comp.meshResource;
				gfx_Object.bindlessGlobalTextureIndex_Albedo = m_comp.albedoID;
				gfx_Object.bindlessGlobalTextureIndex_Normal = m_comp.normalID;
				gfx_Object.submesh = m_comp.meshInfo.submeshBits;

				if (gfx_Object.bones.size() != m_comp.num_bones)
					gfx_Object.bones.resize(m_comp.num_bones);

				//TODO
				//ModelFileResource* model = FindModel_via_modelID(models, m_comp.meshResource);
				//assert(model->skeleton);	//should have skeleton
				//assert(model->skeleton->m_boneNodes);	//should have bones
				//
				//oo::GameObject go{ entity,*scene };
				//AssignGraphicsWorldID_to_Bones_Recursively(go, model, model->skeleton->m_boneNodes);
			});

		AssignGraphicsWorldID_to_BoneComponents();
	}

	void RecurseChildren_AssignGraphicsWorldID_to_BoneComponents(GameObject obj, uint32_t graphicsID, UUID uid)
	{
		auto children = obj.GetChildren();
		if (children.empty()) return;

		for (auto& child : children)
		{
			if (child.HasComponent<SkinMeshBoneComponent>() == false) continue;

			auto& bonecomp = child.GetComponent<SkinMeshBoneComponent>();
			if (bonecomp.root_bone_object != uid) continue;

			bonecomp.graphicsWorld_ID = graphicsID;
		}
	}

	void SkinMeshRendererSystem::AssignGraphicsWorldID_to_BoneComponents()
	{
		static Ecs::Query skin_mesh_query = Ecs::make_query<SkinMeshRendererComponent, TransformComponent>();
		//static Ecs::Query skin_bone_mesh_query = Ecs::make_query<SkinMeshBoneComponent, TransformComponent>();

		scene->GetWorld().for_each_entity(skin_mesh_query,
			[&](Ecs::EntityID entity)
			{
				oo::GameObject go{ entity,*scene };

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


				RecurseChildren_AssignGraphicsWorldID_to_BoneComponents(the_one_with_bones, graphicsID, go.GetComponent<GameObjectComponent>().Id);
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

		Initialize(meshComp, transform_component, go_component);
	}
	void SkinMeshRendererSystem::OnMeshRemove(Ecs::ComponentEvent<SkinMeshRendererComponent>* evnt)
	{
		auto& comp = evnt->component; 
		scene->DestroyGraphicsInstance(comp.graphicsWorld_ID);
		// remove graphics id to uuid of gameobject
		//m_graphicsIdToUUID.erase(comp.GraphicsWorldID);
	}

	void SkinMeshRendererSystem::Initialize(SkinMeshRendererComponent& renderComp, TransformComponent& transformComp, GameObjectComponent& goComp)
	{

		renderComp.graphicsWorld_ID = scene->CreateGraphicsInstance(goComp.Id);

		//update initial position
		auto& graphics_object = m_graphicsWorld->GetObjectInstance(renderComp.graphicsWorld_ID);
		graphics_object.localToWorld = transformComp.GetGlobalMatrix();
		graphics_object.flags = ObjectInstanceFlags::SKINNED | ObjectInstanceFlags::RENDER_ENABLED;
		graphics_object.bones.resize(renderComp.num_bones);
		renderComp.gfx_Object = &graphics_object;
	}
}
