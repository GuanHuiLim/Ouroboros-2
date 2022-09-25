#pragma once
#include "RendererComponent.h"
#include "LightComponent.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"

namespace oo
{
	class MeshRendererSystem : public Ecs::System
	{
	private:
		GraphicsWorld* m_graphicsWorld{nullptr};
		//Ecs::ECSWorld* m_world{nullptr};
	public:
		MeshRendererSystem(GraphicsWorld* graphicsWorld);

		void Init();

		void OnLightAssign(Ecs::ComponentEvent<LightingComponent>* evnt);
		void OnLightRemove(Ecs::ComponentEvent<LightingComponent>* evnt);

		void OnMeshAssign(Ecs::ComponentEvent<MeshRendererComponent>* evnt);
		void OnMeshRemove(Ecs::ComponentEvent<MeshRendererComponent>* evnt);

		//void Init(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld);

		virtual void Run(Ecs::ECSWorld* world) override;
	};
}
