#pragma once
#include "RendererComponent.h"
#include "Archetypes_Ecs/src/Archetype.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"
namespace oo
{
	class MeshRendererSystem
	{
	private:
		GraphicsWorld* graphicsWorld;

	public:
		void AssignObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt);

		void ReleaseObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt);

		void Init(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld);

		void Run(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld);
	};
}
