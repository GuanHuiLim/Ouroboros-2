#pragma once
#include "SkinRendererComponent.h"
#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"

namespace oo
{
	class SkinMeshRendererSystem : public Ecs::System
	{
	private:
		GraphicsWorld* m_graphicsWorld{nullptr};
		oo::Scene* scene{nullptr};

		std::unordered_map<UUID, glm::mat4> root_bone_inverse_map{};
	public:
		SkinMeshRendererSystem(GraphicsWorld* graphicsWorld, oo::Scene* _scene);
		
		void Init();

		virtual void Run(Ecs::ECSWorld* world) override;

		void PostLoadScene();
	private:
		void AssignGraphicsWorldID_to_BoneComponents();
		void OnMeshAssign(Ecs::ComponentEvent<SkinMeshRendererComponent>* evnt);
		void OnMeshRemove(Ecs::ComponentEvent<SkinMeshRendererComponent>* evnt);

		void Initialize(SkinMeshRendererComponent& renderComp, TransformComponent& transformComp, GameObjectComponent& goComp);
	};
}
