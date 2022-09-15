#include "pch.h"
#include "RendererSystem.h"

#include <rttr/registration>
namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<MeshRendererComponent>("MeshRendererComponent")
			.property_readonly("Model handle", &MeshRendererComponent::model_handle);
	}


	void oo::MeshRendererSystem::AssignObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
	{
		auto& comp = evnt->component;
		comp.graphicsWorld_ID = graphicsWorld->CreateObjectInstance();
		//HARDCODED AS CUBE, TO BE REMOVED LATER
		comp.model_handle = 0;

	}

	void oo::MeshRendererSystem::ReleaseObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
	{
		auto& comp = evnt->component;
		//comp.graphicsWorld_ID = graphicsWorld->RemoveObjectInstance();
	}

	void oo::MeshRendererSystem::Init(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld)
	{
		this->graphicsWorld = graphicsWorld;

		world->SubscribeOnAddComponent<MeshRendererSystem, MeshRendererComponent>(
			this, &MeshRendererSystem::AssignObjectInstance);

		world->SubscribeOnRemoveComponent<MeshRendererSystem, MeshRendererComponent>(
			this, &MeshRendererSystem::ReleaseObjectInstance);
	}

	void oo::MeshRendererSystem::Run(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld)
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
}

