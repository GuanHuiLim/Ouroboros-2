#include "pch.h"
#include "SkinRendererSystem.h"

namespace oo
{
	SkinMeshRendererSystem::SkinMeshRendererSystem(GraphicsWorld* graphicsWorld)
		: m_graphicsWorld{ graphicsWorld }
	{
	}
	void SkinMeshRendererSystem::Init()
	{
	}
	void SkinMeshRendererSystem::Run(Ecs::ECSWorld* world)
	{
		static Ecs::Query skin_mesh_query = []()
		{
			Ecs::Query query;
			return query.with<SkinMeshRendererComponent, TransformComponent>().build();
		}();
		static Ecs::Query skin_bone_mesh_query = []()
		{
			Ecs::Query query;
			return query.with<SkinMeshBoneComponent, TransformComponent>().build();
		}();

		world->for_each(skin_bone_mesh_query,
			[&](SkinMeshBoneComponent& boneComp, TransformComponent& transformComp)
			{
				//do nothing if transform did not change
				if (transformComp.HasChanged() == false) return;


				auto& gfx_Object = m_graphicsWorld->GetObjectInstance(boneComp.graphicsWorld_ID);
				gfx_Object.localToWorld = transformComp.GetGlobalMatrix();
			});

		world->for_each(skin_mesh_query, 
			[&](MeshRendererComponent& m_comp, TransformComponent& transformComp)
			{
				//do nothing if transform did not change
				auto& actualObject = m_graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);

				if (transformComp.HasChanged())
					actualObject.localToWorld = transformComp.GetGlobalMatrix();
			});

		
	}
}
