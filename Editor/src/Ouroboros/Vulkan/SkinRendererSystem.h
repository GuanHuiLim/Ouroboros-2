#pragma once
#include "SkinRendererComponent.h"
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

		void PostLoadScene(oo::Scene& scene);
	private:
		void AssignGraphicsWorldID_to_BoneComponents(oo::Scene& scene);
		void OnMeshAssign(Ecs::ComponentEvent<SkinMeshRendererComponent>* evnt);
	};
}
