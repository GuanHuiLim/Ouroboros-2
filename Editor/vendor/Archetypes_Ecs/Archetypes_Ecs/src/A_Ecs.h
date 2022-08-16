#pragma once
#include "EcsUtils.h"
#include "Component.h"
#include "Archetype.h"

#include "Query.h"
#include "World.h"
#include "System.h"
#include "Wrapper.h"
namespace Ecs
{
	template<typename C>
	struct CachedRef
	{
		C* get_from(IECSWorld* world, EntityID target);

		C* pointer;
		EnityToChunk storage;
	};



	template<typename C>
	C* CachedRef<C>::get_from(IECSWorld* world, EntityID target)
	{
		if (world->entities[target.index] != storage) {
			pointer = &world->get_component<C>(target);
			storage = world->entities[target.index];
		}
		return pointer;
	}

	template<typename T>
	inline auto get_chunk_array(DataChunk* chunk) {

		using ActualT = ::std::remove_reference_t<T>;

		if constexpr (std::is_same<ActualT, EntityID>::value)
		{
			EntityID* ptr = ((EntityID*)chunk);
			return ComponentArray<EntityID>(ptr, chunk);
		}
		else {
			constexpr TypeHash hash = ComponentInfo::build_hash<ActualT>();

			for (auto cmp : chunk->header.componentList->components) {
				if (cmp.hash == hash)
				{
					void* ptr = (void*)((byte*)chunk + cmp.chunkOffset);

					return ComponentArray<ActualT>(ptr, chunk);
				}
			}


			return ComponentArray<ActualT>();
		}
	}
}

#include <algorithm>
#include <concepts>
namespace Ecs::internal
{

	//forward declarations
	inline int insert_entity_in_chunk(DataChunk* chunk, EntityID EID, bool bInitializeConstructors = true);
	inline EntityID erase_entity_in_chunk(DataChunk* chunk, uint16_t index);
	inline DataChunk* build_chunk(ComponentCombination* cmpList);


	template<typename... Type>
	struct type_list {};

	template<typename Class, typename Ret, typename... Args>
	type_list<Args...> args(Ret(Class::*)(Args...) const);

	template<typename T>
	static const ComponentInfo* get_ComponentInfo() {
		static const ComponentInfo* mt = []() {
			constexpr size_t name_hash = ComponentInfo::build_hash<T>().name_hash;

			auto type = componentInfo_map.find(name_hash);
			if (type == componentInfo_map.end()) {
				/*constexpr*/ ComponentInfo newtype = ComponentInfo::build<T>();
				componentInfo_map[name_hash] = newtype;
			}
			return &componentInfo_map[name_hash];
		}();
		return mt;
	}


	//reorder archetype with the fullness
	inline void set_chunk_full(DataChunk* chunk) {

		Archetype* arch = chunk->header.archetype;
		if (arch) {
			arch->full_chunks++;

			//do it properly later
			int archsize = arch->componentList->chunkCapacity;

			std::partition(arch->chunks.begin(), arch->chunks.end(), [archsize](DataChunk* cnk) {
				return cnk->header.last == archsize;
				});
		}


	}
	//reorder archetype with the fullness
	inline void set_chunk_partial(DataChunk* chunk) {
		Archetype* arch = chunk->header.archetype;
		arch->full_chunks--;

		//do it properly later

		int archsize = arch->componentList->chunkCapacity;

		std::partition(arch->chunks.begin(), arch->chunks.end(), [archsize](DataChunk* cnk) {
			return cnk->header.last == archsize;
			});
	}

	inline ComponentCombination* build_component_list(const ComponentInfo** types, size_t count) {
		ComponentCombination* list = new ComponentCombination();

		int compsize = sizeof(EntityID);
		for (size_t i = 0; i < count; i++) {

			compsize += types[i]->size;
		}

		size_t availibleStorage = sizeof(DataChunk::storage);
		//2 less than the real count to account for sizes and give some slack
		size_t itemCount = (availibleStorage / compsize) - 2;

		uint32_t offsets = sizeof(DataChunkHeader);
		offsets += static_cast<uint32_t>(sizeof(EntityID) * itemCount);

		for (size_t i = 0; i < count; i++) {
			const ComponentInfo* type = types[i];

			if (type->align != 0) {
				//align properly
				size_t remainder = offsets % type->align;
				size_t oset = type->align - remainder;
				offsets += static_cast<uint32_t>(oset);
			}

			list->components.push_back({ type,type->hash,offsets });

			if (type->align != 0) {
				offsets += type->size * (itemCount);
			}

		}

		//implement proper size handling later
		assert(offsets <= BLOCK_MEMORY_16K);

		list->chunkCapacity = itemCount;

		return list;
	}






	inline size_t build_signature(const ComponentInfo** types, size_t count) {
		size_t and_hash = 0;
		//for (auto m : types)
		for (int i = 0; i < count; i++)
		{
			//consider if the fancy hash is needed, there is a big slowdown
			//size_t keyhash = ash_64_fnv1a(&types[i]->name_hash, sizeof(size_t));
			//size_t keyhash = types[i]->name_hash;

			and_hash ^= types[i]->hash.matcher_hash;
			//and_hash |=(uint64_t)0x1L << (uint64_t)((types[i]->name_hash) % 63L);
		}
		return and_hash;
	}

	inline DataChunk* create_chunk_for_archetype(Archetype* arch) {
		DataChunk* chunk = build_chunk(arch->componentList);

		chunk->header.archetype = arch;
		arch->chunks.push_back(chunk);
		return chunk;
	}
	inline void delete_chunk_from_archetype(DataChunk* chunk) {
		Archetype* owner = chunk->header.archetype;
		DataChunk* backChunk = owner->chunks.back();

		if (backChunk != chunk) {
			for (int i = 0; i < owner->chunks.size(); i++) {
				if (owner->chunks[i] == chunk) {
					owner->chunks[i] = backChunk;
				}
			}
		}
		owner->chunks.pop_back();
		delete chunk;

	}
	inline bool compare_metatypes(const ComponentInfo* A, const ComponentInfo* B) {
		//return A->name_hash < B->name_hash;
		return A < B;
	}
	inline size_t join_metatypes(const ComponentInfo* ATypes[], size_t Acount, const ComponentInfo* BTypes[], size_t Bcount, const ComponentInfo* output[]) {

		const ComponentInfo** AEnd = ATypes + Acount;
		const ComponentInfo** BEnd = BTypes + Bcount;

		const ComponentInfo** A = ATypes;
		const ComponentInfo** B = BTypes;
		const ComponentInfo** C = output;

		while (true)
		{
			if (A == AEnd) {
				std::copy(B, BEnd, C);
			}
			if (B == BEnd) {
				std::copy(A, AEnd, C);
			}

			if (*A < *B) { *C = *A; ++A; }
			else if (*B < *A) { *C = *B; ++B; }
			else { *C = *A; ++A; ++B; }
			++C;
		}

		return C - output;
	}

	inline void sort_ComponentInfos(const ComponentInfo** types, size_t count) {
		std::sort(types, types + count, [](const ComponentInfo* A, const ComponentInfo* B) {
			return compare_metatypes(A, B);
			});
	}
	inline bool is_sorted(const ComponentInfo** types, size_t count) {
		for (int i = 0; i < count - 1; i++) {
			if (types[i] > types[i + 1]) {
				return false;
			}
		}
		return true;
	}

	inline Archetype* find_or_create_archetype(IECSWorld* world, const ComponentInfo** types, size_t count) {
		const ComponentInfo* temporalComponentInfoArray[MAX_COMPONENTS];
		assert(count < MAX_COMPONENTS);

		const ComponentInfo** typelist;

		if (false) {//!is_sorted(types, count)) {
			for (int i = 0; i < count; i++) {
				temporalComponentInfoArray[i] = types[i];

			}
			sort_ComponentInfos(temporalComponentInfoArray, count);
			typelist = temporalComponentInfoArray;
		}
		else {
			typelist = types;
		}

		const uint64_t matcher = build_signature(typelist, count);

		//try in the hashmap
		auto iter = world->archetype_signature_map.find(matcher);
		if (iter != world->archetype_signature_map.end()) {

			auto& archvec = iter->second;//world->archetype_signature_map[matcher];
			for (int i = 0; i < archvec.size(); i++) {

				auto componentList = archvec[i]->componentList;
				int ccount = componentList->components.size();
				if (ccount == count) {
					for (int j = 0; j < ccount; j++) {

						if (componentList->components[j].type != typelist[j])
						{
							//mismatch, inmediately continue
							goto contA;
						}
					}

					//everything matched. Found. Return inmediately
					return archvec[i];
				}

			contA:;
			}
		}

		//not found, create a new one

		Archetype* newArch = new Archetype();

		newArch->full_chunks = 0;
		newArch->componentList = build_component_list(typelist, count);
		newArch->componentHash = matcher;
		newArch->ownerWorld = world;
		world->archetypes.push_back(newArch);
		world->archetypeSignatures.push_back(matcher);
		world->archetype_signature_map[matcher].push_back(newArch);

		//we want archs to allways have 1 chunk at least, create initial
		create_chunk_for_archetype(newArch);
		return newArch;
	}


	inline EntityID allocate_entity(IECSWorld* world) {
		EntityID newID;
		if (world->dead_entities == 0) {
			int index = world->entities.size();

			EnityToChunk newStorage;
			newStorage.chunk = nullptr;
			newStorage.chunkIndex = 0;
			newStorage.generation = 1;

			world->entities.push_back(newStorage);

			newID.generation = 1;
			newID.index = index;
		}
		else {
			int index = world->deletedEntities.back();
			world->deletedEntities.pop_back();

			world->entities[index].generation++;

			newID.generation = world->entities[index].generation;
			newID.index = index;
			world->dead_entities--;
		}

		world->live_entities++;
		return newID;
	}

	inline bool is_entity_valid(IECSWorld* world, EntityID id) {
		//index check
		if (world->entities.size() > id.index && id.index >= 0) {

			//generation check
			if (id.generation != 0 && world->entities[id.index].generation == id.generation)
			{
				return true;
			}
		}
		return false;
	}
	inline void deallocate_entity(IECSWorld* world, EntityID id) {

		//todo add valid check
		world->deletedEntities.push_back(id.index);
		world->entities[id.index].generation++;
		world->entities[id.index].chunk = nullptr;
		world->entities[id.index].chunkIndex = 0;

		world->live_entities--;
		world->dead_entities++;
	}

	inline void destroy_entity(IECSWorld* world, EntityID id) {
		assert(is_entity_valid(world, id));
		erase_entity_in_chunk(world->entities[id.index].chunk, world->entities[id.index].chunkIndex);
		deallocate_entity(world, id);
	}
	inline DataChunk* find_free_chunk(Archetype* arch) {
		DataChunk* targetChunk = nullptr;
		if (arch->chunks.size() == 0) {
			targetChunk = create_chunk_for_archetype(arch);
		}
		else {
			targetChunk = arch->chunks[arch->chunks.size() - 1];
			//chunk is full, create a new one
			if (targetChunk->header.last == arch->componentList->chunkCapacity) {
				targetChunk = create_chunk_for_archetype(arch);
			}
		}
		return targetChunk;
	}


	inline void move_entity_to_archetype(Archetype* newarch, EntityID id, bool bInitializeConstructors = true) {

		//insert into new chunk
		DataChunk* oldChunk = newarch->ownerWorld->entities[id.index].chunk;
		DataChunk* newChunk = find_free_chunk(newarch);

		int newindex = insert_entity_in_chunk(newChunk, id, bInitializeConstructors);
		int oldindex = newarch->ownerWorld->entities[id.index].chunkIndex;

		auto oldNcomps = oldChunk->header.componentList->components.size();
		auto newNcomps = newChunk->header.componentList->components.size();

		auto& oldClist = oldChunk->header.componentList;
		auto& newClist = newChunk->header.componentList;

		//copy all data from old chunk into new chunk
		//bad iteration, fix later

		struct Merge {
			int msize;
			int idxOld;
			int idxNew;
			const ComponentInfo* info;
		};
		int mergcount = 0;
		Merge mergarray[MAX_COMPONENTS];

		for (auto i = 0ull; i < oldNcomps; i++) {
			const ComponentInfo* mtCp1 = oldClist->components[i].type;
			if (!mtCp1->is_empty()) {
				for (int j = 0; j < newNcomps; j++) {
					const ComponentInfo* mtCp2 = newClist->components[j].type;

					//pointers are stable
					if (mtCp2 == mtCp1) {
						mergarray[mergcount].idxNew = j;
						mergarray[mergcount].idxOld = i;
						mergarray[mergcount].msize = mtCp1->size;
						mergarray[mergcount].info = mtCp2;
						mergcount++;
					}
				}
			}
		}

		for (int i = 0; i < mergcount; i++) {
			//const ComponentInfo* mtCp1 = mergarray[i].mtype;

			//pointer for old location in old chunk
			void* ptrOld = (void*)((byte*)oldChunk + oldClist->components[mergarray[i].idxOld].chunkOffset + 
				(mergarray[i].msize * oldindex));

			//pointer for new location in new chunk
			void* ptrNew = (void*)((byte*)newChunk + newClist->components[mergarray[i].idxNew].chunkOffset + 
				(mergarray[i].msize * newindex));

			//memcopy component data from old to new
			//memcpy(ptrNew, ptrOld, mergarray[i].msize);
			//move construct
			//mergarray[i].info->move_constructor(ptrNew, ptrOld);
			
			//actualy screw that, move assign
			mergarray[i].info->move_assignment(ptrNew, ptrOld);
		}

		//delete entity from old chunk
		erase_entity_in_chunk(oldChunk, oldindex);

		//assign entity chunk data
		newarch->ownerWorld->entities[id.index].chunk = newChunk;
		newarch->ownerWorld->entities[id.index].chunkIndex = newindex;
	}

	inline void copy_entity_data_in_archetype(Archetype* arch, EntityID copy, EntityID original) {
		//insert into new chunk
		/*DataChunk* oldChunk = newarch->ownerWorld->entities[id.index].chunk;
		DataChunk* newChunk = find_free_chunk(newarch);

		int newindex = insert_entity_in_chunk(newChunk, id, bInitializeConstructors);
		int oldindex = newarch->ownerWorld->entities[id.index].chunkIndex;*/

		//auto oldNcomps = oldChunk->header.componentList->components.size();
		//auto newNcomps = newChunk->header.componentList->components.size();

		//auto& oldClist = oldChunk->header.componentList;
		//auto& newClist = newChunk->header.componentList;
		DataChunk* originalChunk = arch->ownerWorld->entities[original.index].chunk;
		DataChunk* copyChunk	 = arch->ownerWorld->entities[copy.index].chunk;

		auto& originalClist = originalChunk->header.componentList;
		auto& copyClist		= copyChunk->header.componentList;

		int originalindex = arch->ownerWorld->entities[original.index].chunkIndex;
		int copyindex	  = arch->ownerWorld->entities[copy.index].chunkIndex;

		auto& componentList = arch->componentList->components;

		//copy all data from old chunk into new chunk
		//bad iteration, fix later

		struct Info {
			int msize;
			const ComponentInfo* info;
		};
		auto infocount = componentList.size();
		Info infoarray[MAX_COMPONENTS];

		//for (auto i = 0ull; i < oldNcomps; i++) {
		//	const ComponentInfo* mtCp1 = oldClist->components[i].type;
		//	if (!mtCp1->is_empty()) {
		//		for (int j = 0; j < newNcomps; j++) {
		//			const ComponentInfo* mtCp2 = newClist->components[j].type;

		//			//pointers are stable
		//			if (mtCp2 == mtCp1) {
		//				mergarray[mergcount].idxNew = j;
		//				mergarray[mergcount].idxOld = i;
		//				mergarray[mergcount].msize = mtCp1->size;
		//				mergarray[mergcount].info = mtCp2;
		//				mergcount++;
		//			}
		//		}
		//	}
		//}

		for (int i = 0; i < infocount; i++) {
			//const ComponentInfo* mtCp1 = mergarray[i].mtype;

			//pointer for old location in old chunk
			void* ptrOriginal = (void*)((byte*)originalChunk + componentList[i].chunkOffset +
				(componentList[i].type->size * originalindex));

			//pointer for new location in new chunk
			void* ptrCopy = (void*)((byte*)copyChunk + componentList[i].chunkOffset +
				(componentList[i].type->size * copyindex));

			//memcopy component data from old to new
			//memcpy(ptrNew, ptrOld, mergarray[i].msize);
			//copy construct
			componentList[i].type->copy_constructor(ptrCopy, ptrOriginal);
		}
	
	}

	inline void set_entity_archetype(Archetype* arch, EntityID id, bool bInitializeConstructors = true) {

		//if chunk is null, we are a empty entity
		if (arch->ownerWorld->entities[id.index].chunk == nullptr) {

			DataChunk* targetChunk = find_free_chunk(arch);

			int index = insert_entity_in_chunk(targetChunk, id, bInitializeConstructors);
			arch->ownerWorld->entities[id.index].chunk = targetChunk;
			arch->ownerWorld->entities[id.index].chunkIndex = index;
		}
		else {
			move_entity_to_archetype(arch, id, true);
		}
	}
	inline EntityID create_entity_with_archetype(Archetype* arch, bool bInitializeConstructors = true) {
		IECSWorld* world = arch->ownerWorld;

		EntityID newID = allocate_entity(world);

		set_entity_archetype(arch, newID, bInitializeConstructors);

		return newID;
	}

	inline EntityID duplicate_entity_with_archetype(Archetype* arch, EntityID id) {
		EntityID newID = create_entity_with_archetype(arch,false);

		copy_entity_data_in_archetype(arch, newID, id);

		return newID;
	}


	inline Archetype* get_entity_archetype(IECSWorld* world, EntityID id)
	{
		assert(is_entity_valid(world, id));

		return world->entities[id.index].chunk->header.archetype;
	}


	template<typename C>
	bool has_component(IECSWorld* world, EntityID id);

	template<typename C>
	C& get_entity_component(IECSWorld* world, EntityID id);

	template<typename C>
	void add_component_to_entity(IECSWorld* world, EntityID id, C&& comp);

	template<typename C>
	void remove_component_from_entity(IECSWorld* world, EntityID id);

	template<typename F>
	void iterate_matching_archetypes(IECSWorld* world, const IQuery& query, F&& function) {

		for (int i = 0; i < world->archetypeSignatures.size(); i++)
		{
			//if there is a good match, doing an and not be 0
			uint64_t includeTest = world->archetypeSignatures[i] & query.require_matcher;

			//implement later
			uint64_t excludeTest = world->archetypeSignatures[i] & query.exclude_matcher;


			if (includeTest != 0) {

				auto componentList = world->archetypes[i]->componentList;

				//might match an excluded component, check here
				if (excludeTest != 0) {

					bool invalid = false;
					//dumb algo, optimize later					
					for (int mtA = 0; mtA < query.exclude_comps.size(); mtA++) {

						for (auto cmp : componentList->components) {

							if (cmp.type->hash == query.exclude_comps[mtA]) {
								//any check and we out
								invalid = true;
								break;
							}
						}
						if (invalid) {
							break;
						}
					}
					if (invalid) {
						continue;
					}
				}

				//dumb algo, optimize later
				int matches = 0;
				for (int mtA = 0; mtA < query.require_comps.size(); mtA++) {

					for (auto cmp : componentList->components) {

						if (cmp.type->hash == query.require_comps[mtA]) {
							matches++;
							break;
						}
					}
				}
				//all perfect
				if (matches == query.require_comps.size()) {

					function(world->archetypes[i]);
				}
			}


		}

	}



	template<typename C>
	C& get_entity_component(IECSWorld* world, EntityID id)
	{

		EnityToChunk& storage = world->entities[id.index];

		auto acrray = get_chunk_array<C>(storage.chunk);
		assert(acrray.chunkOwner != nullptr);
		return acrray[storage.chunkIndex];
	}


	template<typename C>
	bool has_component(IECSWorld* world, EntityID id)
	{
		EnityToChunk& storage = world->entities[id.index];

		auto acrray = get_chunk_array<C>(storage.chunk);
		return acrray.chunkOwner != nullptr;
	}
	template<typename C>
	void add_component_to_entity(IECSWorld* world, EntityID id)
	{
		const ComponentInfo* temporalComponentInfoArray[MAX_COMPONENTS];

		const ComponentInfo* type = get_ComponentInfo<C>();


		Archetype* oldarch = get_entity_archetype(world, id);
		ComponentCombination* oldlist = oldarch->componentList;
		bool typeFound = false;
		auto length = oldlist->components.size();
		//fill up component info array
		for (int i = 0; i < oldlist->components.size(); i++) {
			temporalComponentInfoArray[i] = oldlist->components[i].type;

			//the pointers for metatypes are always fully stable
			
			//if the component is somehow already added, we do nothing
			if (temporalComponentInfoArray[i] == type) {
				typeFound = true;
			}
		}

		//move the entity into the new archetype
		Archetype* newArch = oldarch;
		if (!typeFound) {

			temporalComponentInfoArray[length] = type;
			sort_ComponentInfos(temporalComponentInfoArray, length + 1ul);
			length++;


			newArch = find_or_create_archetype(world, temporalComponentInfoArray, length);



			set_entity_archetype(newArch, id);
		}

	}
	template<typename C>
	void add_component_to_entity(IECSWorld* world, EntityID id, C& comp)
	{
		const ComponentInfo* type = get_ComponentInfo<C>();

		add_component_to_entity<C>(world, id);


		//optimize later
		if (type->is_empty() == false) {
			get_entity_component<C>(world, id) = comp;
		}
	}

	template<typename C>
	void remove_component_from_entity(IECSWorld* world, EntityID id)
	{
		const ComponentInfo* temporalComponentInfoArray[MAX_COMPONENTS];

		const ComponentInfo* type = get_ComponentInfo<C>();

		Archetype* oldarch = get_entity_archetype(world, id);
		ComponentCombination* oldlist = oldarch->componentList;
		bool typeFound = false;
		int lenght = oldlist->components.size();
		for (int i = 0; i < lenght; i++) {
			temporalComponentInfoArray[i] = oldlist->components[i].type;

			//the pointers for metatypes are allways fully stable
			if (temporalComponentInfoArray[i] == type) {
				typeFound = true;
				//swap last
				temporalComponentInfoArray[i] = oldlist->components[lenght - 1].type;
			}
		}

		Archetype* newArch = oldarch;
		if (typeFound) {

			lenght--;
			sort_ComponentInfos(temporalComponentInfoArray, lenght);

			newArch = find_or_create_archetype(world, temporalComponentInfoArray, lenght);

			set_entity_archetype(newArch, id);
		}
	}


	//template<typename A, typename B, typename C, typename D, typename Func>
	//void entity_chunk_iterate(DataChunk* chnk, Func&& function) {
	//
	//	auto array0 = get_chunk_array<A>(chnk);
	//	auto array1 = get_chunk_array<B>(chnk);
	//	auto array2 = get_chunk_array<C>(chnk);
	//	auto array3 = get_chunk_array<D>(chnk);
	//
	//	assert(array0.chunkOwner == chnk);
	//	assert(array1.chunkOwner == chnk);
	//	assert(array2.chunkOwner == chnk);
	//	assert(array3.chunkOwner == chnk);
	//	for (int i = chnk->header.last - 1; i >= 0; i--) {
	//		function(array0[i], array1[i], array2[i], array3[i]);
	//	}
	//}

	//by skypjack
	template<typename... Args, typename Func>
	void entity_chunk_iterate(DataChunk* chnk, Func&& function) {
		auto tup = std::make_tuple(get_chunk_array<Args>(chnk)...);
#ifndef NDEBUG
		(assert(std::get<decltype(get_chunk_array<Args>(chnk))>(tup).chunkOwner == chnk), ...);
#endif

		for (int i = chnk->header.last - 1; i >= 0; i--) {
			function(std::get<decltype(get_chunk_array<Args>(chnk))>(tup)[i]...);
		}
	}


	template<typename ...Args, typename Func>
	void unpack_chunk(type_list<Args...> types, DataChunk* chunk, Func&& function) {
		entity_chunk_iterate<Args...>(chunk, function);
	}
	template<typename ...Args>
	IQuery& unpack_querywith(type_list<Args...> types, IQuery& query) {
		return query.with<Args...>();
	}





	inline int insert_entity_in_chunk(DataChunk* chunk, EntityID EID, bool bInitializeConstructors) {
		int index = -1;

		ComponentCombination* cmpList = chunk->header.componentList;

		if (chunk->header.last < cmpList->chunkCapacity) {

			index = chunk->header.last;
			chunk->header.last++;

			if (bInitializeConstructors) {
				//initialize component
				for (auto& cmp : cmpList->components) {
					const ComponentInfo* mtype = cmp.type;

					if (!mtype->is_empty()) {
						void* ptr = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * index));

						mtype->constructor(ptr);
					}
				}
			}


			//insert eid
			EntityID* eidptr = ((EntityID*)chunk);
			eidptr[index] = EID;

			//if full, reorder it on archetype
			if (chunk->header.last == cmpList->chunkCapacity) {
				set_chunk_full(chunk);
			}
		}

		return index;
	}

	//returns ID of the moved entity
	inline EntityID erase_entity_in_chunk(DataChunk* chunk, uint16_t index) {

		ComponentCombination* cmpList = chunk->header.componentList;

		bool bWasFull = chunk->header.last == cmpList->chunkCapacity;
		assert(chunk->header.last > index);

		bool bPop = chunk->header.last > 1 && index != (chunk->header.last - 1);
		int popIndex = chunk->header.last - 1;

		chunk->header.last--;

		//clear and pop last
		for (auto& cmp : cmpList->components) {
			const ComponentInfo* mtype = cmp.type;

			if (!mtype->is_empty()) {
				void* ptr = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * index));

				mtype->destructor(ptr);

				if (bPop) {
					void* ptrPop = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * popIndex));
					memcpy(ptr, ptrPop, mtype->size);
				}
			}
		}

		EntityID* eidptr = ((EntityID*)chunk);
		eidptr[index] = EntityID{};


		if (chunk->header.last == 0) {
			delete_chunk_from_archetype(chunk);
		}
		//if chunk used to be full, its not anymore so tell the archetype
		else if (bWasFull) {
			set_chunk_partial(chunk);
		}

		if (bPop) {
			chunk->header.archetype->ownerWorld->entities[eidptr[popIndex].index].chunkIndex = index;
			eidptr[index] = eidptr[popIndex];

			return eidptr[index];
		}
		else {
			return EntityID{};
		}
	}

	inline DataChunk* build_chunk(ComponentCombination* cmpList) {

		DataChunk* chunk = new DataChunk();
		chunk->header.last = 0;
		chunk->header.componentList = cmpList;

		return chunk;
	}
}

namespace Ecs
{
	inline IECSWorld::IECSWorld()
	{
		Archetype* nullArch = new Archetype();

		nullArch->full_chunks = 0;
		nullArch->componentList = internal::build_component_list(nullptr, 0);
		nullArch->componentHash = 0;
		nullArch->ownerWorld = this;

		archetypes.push_back(nullArch);

		archetypeSignatures.push_back(0);

		archetype_signature_map[0].push_back(nullArch);

		//we want archs to allways have 1 chunk at least, create initial
		internal::create_chunk_for_archetype(nullArch);
	}

	template<typename Container>
	int IECSWorld::gather_chunks(IQuery& query, Container& container)
	{
		int count = 0;
		internal::iterate_matching_archetypes(this, query, [&](Archetype* arch) {

			for (auto chnk : arch->chunks) {
				count++;
				container.push_back(chnk);
			}
			});
		return count;
	}

	inline void IECSWorld::destroy(EntityID eid)
	{
		internal::destroy_entity(this, eid);
	}

	template<typename Func>
	void IECSWorld::for_each(IQuery& query, Func&& function)
	{
		using params = decltype(internal::args(&Func::operator()));

		internal::iterate_matching_archetypes(this, query, [&](Archetype* arch) {

			for (auto chnk : arch->chunks) {

				internal::unpack_chunk(params{}, chnk, function);
			}
			});
	}
	template<typename Func>
	void IECSWorld::for_each(Func&& function)
	{
		using params = decltype(internal::args(&Func::operator()));

		IQuery query;
		internal::unpack_querywith(params{}, query).build();

		for_each<Func>(query, std::move(function));
	}

	template<typename C>
	inline void IECSWorld::add_component(EntityID id, C& comp)
	{
		internal::add_component_to_entity<C>(this, id, comp);
	}

	template<typename C>
	void IECSWorld::add_component(EntityID id)
	{
		internal::add_component_to_entity<C>(this, id);
	}

	template<typename C>
	inline void IECSWorld::remove_component(EntityID id)
	{
		internal::remove_component_from_entity<C>(this, id);
	}


	template<typename C>
	bool IECSWorld::has_component(EntityID id)
	{
		return internal::has_component<C>(this, id);
	}

	template<typename C>
	C& IECSWorld::get_component(EntityID id)
	{
		return internal::get_entity_component<C>(this, id);
	}

	template<typename C>
	inline C* IECSWorld::set_singleton()
	{
		return set_singleton<C>(C{});
	}

	template<typename C>
	inline C* IECSWorld::set_singleton(C&& singleton)
	{
		constexpr TypeHash type = ComponentInfo::build_hash<C>();

		C* old_singleton = get_singleton<C>();
		if (old_singleton) {
			*old_singleton = singleton;
			return old_singleton;
		}
		else {

			singleton_info_map[type.name_hash] = ComponentInfo::build<C>();
			void* new_singleton = static_cast<void*>(new char[singleton_info_map[type.name_hash].size]);
			//run the default constructor
			singleton_info_map[type.name_hash].constructor(new_singleton);
			singleton_map[type.name_hash] = new_singleton;
			return static_cast<C*>(new_singleton);
		}

	}

	template<typename C>
	inline C* IECSWorld::get_singleton()
	{
		constexpr TypeHash type = ComponentInfo::build_hash<C>();

		auto lookup = singleton_map.find(type.name_hash);
		if (lookup != singleton_map.end()) {
			return (C*)singleton_map[type.name_hash];
		}
		else {
			return nullptr;
		}
	}

	template<typename ...Comps>
	inline EntityID IECSWorld::new_entity()
	{
		Archetype* arch = nullptr;
		//empty component list will use the hardcoded null archetype
		if constexpr (sizeof...(Comps) != 0) {
			static const ComponentInfo* types[] = { internal::get_ComponentInfo<Comps>()... };
			constexpr size_t num = (sizeof(types) / sizeof(*types));

			internal::sort_ComponentInfos(types, num);
			arch = internal::find_or_create_archetype(this, types, num);
		}
		else {
			arch = get_empty_archetype();
		}

		return internal::create_entity_with_archetype(arch);
	}

	

	template<typename S>
	inline S* IECSWorld::Add_System()
	{
		if (system_map.contains(typeid(S).name()))
			return system_map[typeid(S).name()];
		//create the system
		S* system = new S();
		if constexpr (std::derived_from<S, System> == true) {
			system->world = this;
		}
		system_map[typeid(S).name()] = system;
			return system;
	}

	template<typename S>
	inline S* IECSWorld::Get_System()
	{
		if (system_map.contains(typeid(S).name()) == false)
			return nullptr;

		return system_map[typeid(S).name()];
	}
}
