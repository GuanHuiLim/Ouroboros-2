#pragma once
#include "EcsUtils.h"
#include "Archetype.h"

#include <vector>
#include <unordered_map>
namespace Ecs
{

	struct IECSWorld {
		std::vector<EnityToChunk> entities;
		std::vector<EntityID::index_type> deletedEntities;

		std::unordered_map<uint64_t, std::vector<Archetype*>> archetype_signature_map{};
		std::unordered_map<uint64_t, Archetype*> archetype_map{};
		std::vector<Archetype*> archetypes;
		//unique archetype hashes
		std::vector<size_t> archetypeHashes;
		//bytemask hash for checking
		std::vector<size_t> archetypeSignatures;

		std::unordered_map<uint64_t, void*> singleton_map{};
		std::unordered_map<uint64_t, ComponentInfo> singleton_info_map{};

		std::unordered_map<size_t, void*> system_map{};

		int live_entities{ 0 }; //tracks number of active entity IDs
		int dead_entities{ 0 }; //tracks number of dead entity IDs
		inline IECSWorld();
		~IECSWorld();

		//needs push_back(DataChunk*) to work, returns number
		template<typename Container>
		int gather_chunks(IQuery& query, Container& container);

		template<typename Func>
		void for_each(IQuery& query, Func&& function);

		//priority
		template<typename Func>
		void for_each(Func&& function);

		//priority
		template<typename C>
		void add_component(EntityID id, C& comp);
		template<typename C>
		void add_component(EntityID id);
		//priority
		template<typename C>
		void remove_component(EntityID id);
		//priority
		template<typename C>
		bool has_component(EntityID id);
		//priority
		template<typename C>
		C& get_component(EntityID id);

		template<typename C>
		C* set_singleton();
		template<typename C>
		C* set_singleton(C&& singleton);

		template<typename C>
		C* get_singleton();

		template<typename ... Comps>
		inline EntityID new_entity();

		EntityID new_entity(std::vector<uint64_t>const& component_hashes);
		EntityID duplicate_entity(EntityID id);

		std::vector<uint64_t> const componentHashes(EntityID id);

		inline void destroy(EntityID eid);

		Archetype* get_empty_archetype() { return archetypes[0]; };

		template<typename S>
		S* Add_System();

		template<typename S>
		S* Get_System();
	};
}// namespace Ecs