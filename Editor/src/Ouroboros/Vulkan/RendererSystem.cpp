#include "pch.h"
#include "RendererSystem.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>
namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
	registration::class_<MeshRendererComponent>("MeshRendererComponent")
		.property_readonly("Model Handle", &MeshRendererComponent::model_handle)
		.property("Mesh", &MeshRendererComponent::mesh_handle)
			(
				metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
			);
	}


	void oo::MeshRendererSystem::AssignObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
	{
		assert(ecs_world != nullptr); // it should never be nullptr, was the Init funciton called?

		auto& comp = evnt->component;
		comp.graphicsWorld_ID = graphicsWorld->CreateObjectInstance();
		//HARDCODED AS CUBE, TO BE REMOVED LATER
		comp.model_handle = 0;
		
		//update graphics world side
		auto& transform_component = ecs_world->get_component<TransformComponent>(evnt->entityID);
		auto& graphics_object = graphicsWorld->GetObjectInstance(comp.graphicsWorld_ID);
		graphics_object.localToWorld = transform_component.GetGlobalMatrix();
		
	}

	void oo::MeshRendererSystem::ReleaseObjectInstance(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
	{
		auto& comp = evnt->component;
		graphicsWorld->DestroyObjectInstance(comp.graphicsWorld_ID);
	}

	void oo::MeshRendererSystem::Init(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld)
	{
		assert(graphicsWorld != nullptr);	// it should never be nullptr, who's calling this?
		assert(world != nullptr);			// it should never be nullptr, who's calling this?

		this->graphicsWorld = graphicsWorld;
		this->ecs_world = world;

		world->SubscribeOnAddComponent<MeshRendererSystem, MeshRendererComponent>(
			this, &MeshRendererSystem::AssignObjectInstance);

		world->SubscribeOnRemoveComponent<MeshRendererSystem, MeshRendererComponent>(
			this, &MeshRendererSystem::ReleaseObjectInstance);
	}

	void oo::MeshRendererSystem::Run()
	{
		static Ecs::Query query = []() {
			Ecs::Query query;
			return query.with<MeshRendererComponent, TransformComponent>().build();
		}();


		ecs_world->for_each(query, [&](MeshRendererComponent& m_comp, TransformComponent& transformComp) {

			//do nothing if transform did not change
			auto& actualObject = graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);

			if (transformComp.HasChanged())
				actualObject.localToWorld = transformComp.GetGlobalMatrix();
			});
	}
}

