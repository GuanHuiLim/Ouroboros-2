#include "A_Ecs.h"

namespace Ecs
{
	std::unordered_map<uint64_t, ComponentInfo> componentInfo_map{};

	IECSWorld::~IECSWorld()
	{
		//destuctor for all entity components
		for (auto& entt : entities)
		{
			if (entt.chunk == nullptr) continue;
			//erase_entity_in_chunk(oldChunk, oldindex);
			internal::erase_entity_in_chunk(entt.chunk, entt.chunkIndex);
		}



		for (Archetype* arch : archetypes)
		{
			//delete the data chunks
			for (DataChunk* chunk : arch->chunks) {

				delete chunk;
			}
			//rest of the dynamic memory variables
			delete arch->componentList;

			delete arch;
		}


		//delete systems
		for (auto s : system_map)
			s.second.destructor(s.second.system);

		system_map.clear();

		//delete singletons
		for (auto s : singleton_map)
			delete[] s.second;

		singleton_map.clear();
	};

	size_t IECSWorld::get_num_components(EntityID id)
	{
		assert(internal::is_entity_valid(this, id));
		EnityToChunk& storage = entities[id.index];
		assert(storage.chunk != nullptr);

		return storage.chunk->header.componentList->components.size();
	}

	EntityID IECSWorld::new_entity(std::vector<uint64_t> const& component_hashes)
	{
		Archetype* arch = nullptr;
		//empty component list will use the hardcoded null archetype
		if (component_hashes.empty() == false) {
			std::vector<const ComponentInfo*> componentInfos;
			componentInfos.reserve(component_hashes.size());
			for (auto c : component_hashes)
				componentInfos.emplace_back(&componentInfo_map[c]);

			const ComponentInfo** types = &componentInfos[0];
			size_t num = component_hashes.size();

			internal::sort_ComponentInfos(types, num);
			arch = internal::find_or_create_archetype(this, types, num);
		}
		else {
			arch = get_empty_archetype();
		}

		auto entity = internal::create_entity_with_archetype(arch);
		internal::broadcast_add_entity_callback(this, entity);
		return entity;
	}

	EntityID IECSWorld::duplicate_entity(EntityID id)
	{
		assert(internal::is_entity_valid(this,id));
		Archetype* arch = internal::get_entity_archetype(this, id);


		auto entity = internal::duplicate_entity_with_archetype(arch, id);
		internal::broadcast_add_entity_callback(this, entity);
		return entity;
	}


	std::vector<uint64_t> const IECSWorld::componentHashes(EntityID id)
	{
		//if invalid id return nothing
		if (internal::is_entity_valid(this, id) == false) return {};

		std::vector<uint64_t> hashes;
		Archetype* arch = entities[id.index].chunk->header.archetype;
		for (auto& c : arch->componentList->components)
			hashes.emplace_back(c.type->hash.name_hash);

		return hashes;
	}

	void IECSWorld::SubscribeOnAddEntity(FnPtr function)
	{
		onAddEntity_callbacks.Subscribe<EntityEvent>(function);
	}

	void IECSWorld::SubscribeOnDestroyEntity(FnPtr function)
	{
		onDestroyEntity_callbacks.Subscribe<EntityEvent>(function);
	}

	EntityID ECSWorld::new_entity(std::vector<uint64_t> const& component_hashes)
	{
		return world.new_entity(component_hashes);
	}

	EntityID ECSWorld::duplicate_entity(EntityID id)
	{
		return world.duplicate_entity(id);
	}

	std::vector<uint64_t> const ECSWorld::componentHashes(EntityID id)
	{
		return world.componentHashes(id);
	}

	void ECSWorld::destroy(EntityID eid)
	{
		world.destroy(eid);
	}

	void ECSWorld::SubscribeOnAddEntity(FnPtr function)
	{
		world.SubscribeOnAddEntity(function);
	}

	void ECSWorld::SubscribeOnDestroyEntity(FnPtr function)
	{
		world.SubscribeOnDestroyEntity(function);
	}
}


namespace Ecs::testing
{
	void NonMemberFunction(Ecs::EntityEvent* event) {
		auto entity = event->entity;
	};

	void AnotherNonMemberFunction(Ecs::ComponentEvent<Ecs::TestComponent>* event) {
		auto& comp = event->component;
		auto entityID = event->entityID;
	};
	void test()
	{
		class TestClass
		{
		public:
			void MemberFunction(Ecs::EntityEvent* event) {
				auto entity = event->entity;
			};
			void 
				AnotherMemberFunction(Ecs::ComponentEvent<Ecs::TestComponent>* event) {
				auto& comp = event->component;
				auto entityID = event->entityID;
			};
		};

		ECSWorld world;
		world.Add_System<Ecs::internal::TestSystem>();
		world.Get_System<Ecs::internal::TestSystem>();
		world.Run_System<Ecs::internal::TestSystem>();

		TestClass testclass;
		world.SubscribeOnAddEntity(&NonMemberFunction);
		world.SubscribeOnAddEntity(&testclass, &TestClass::MemberFunction);


		world.SubscribeOnDestroyEntity(&NonMemberFunction);
		world.SubscribeOnDestroyEntity(&testclass, &TestClass::MemberFunction);
	

		world.SubscribeOnAddComponent<Ecs::TestComponent>(&AnotherNonMemberFunction);
		world.SubscribeOnAddComponent<TestClass, Ecs::TestComponent>(&testclass, &TestClass::AnotherMemberFunction);


		world.SubscribeOnRemoveComponent<Ecs::TestComponent>(&AnotherNonMemberFunction);
		world.SubscribeOnRemoveComponent<TestClass, Ecs::TestComponent>(&testclass, &TestClass::AnotherMemberFunction);
	
		auto entity = world.new_entity();
		world.add_component<TestComponent>(entity);
		world.remove_component<TestComponent>(entity);
	}
}
//#include "World.cpp"
//#include "EcsCommon.cpp"
