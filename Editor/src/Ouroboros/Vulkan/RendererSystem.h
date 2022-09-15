#pragma once
#include "RendererComponent.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"
namespace oo
{
	class MeshRendererSystem
	{
	private:
		GraphicsWorld* graphicsWorld{nullptr};
		Ecs::ECSWorld* ecs_world{nullptr};
	public:
		void AssignObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt);

		void ReleaseObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt);

		void Init(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld);

		void Run();
	};
}
