#pragma once
#include "RendererComponent.h"
#include "Archetypes_Ecs/src/Archetype.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"
namespace oo
{
	class MeshRendererSystem
	{
		GraphicsWorld* graphicsWorld;



	public:
		void AssignObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
		{
			auto& comp = evnt->component;
			comp.graphicsWorld_ID = graphicsWorld->CreateObjectInstance();
		}

		void ReleaseObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
		{
			auto& comp = evnt->component;
			//comp.graphicsWorld_ID = graphicsWorld->RemoveObjectInstance();
		}

		void Init(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld)
		{
			this->graphicsWorld = graphicsWorld;

			world->SubscribeOnAddComponent<MeshRendererSystem, MeshRendererComponent>(
				this,&MeshRendererSystem::AssignObjectInstance);

			world->SubscribeOnRemoveComponent<MeshRendererSystem, MeshRendererComponent>(
				this, &MeshRendererSystem::ReleaseObjectInstance);
		}

		void Run(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld)
		{
			static Ecs::Query query = []() {
				Ecs::Query query;
				return query.with<MeshRendererComponent, TransformComponent>().build();
			}();


			world->for_each(query, [&](MeshRendererComponent& m_comp, TransformComponent& transformComp) {
				
				//do nothing
				auto& actualObject = graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);

				if (transformComp.HasChanged())
					actualObject.localToWorld = transformComp.GetGlobalMatrix();
				});
		}
	};
}
