#pragma once
#include "RendererComponent.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"

namespace oo
{
	class SkinMeshRendererSystem : public Ecs::System
	{
	private:
		GraphicsWorld* m_graphicsWorld{nullptr};
		//Ecs::ECSWorld* m_world{nullptr};
	public:
		SkinMeshRendererSystem(GraphicsWorld* graphicsWorld);

		void Init();

		virtual void Run(Ecs::ECSWorld* world) override;

	private:
		void OnMeshAssign(Ecs::ComponentEvent<SkinMeshRendererComponent>* evnt);
	};
}
