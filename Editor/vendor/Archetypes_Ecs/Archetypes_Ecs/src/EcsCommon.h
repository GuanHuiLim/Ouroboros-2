/************************************************************************************//*!
\file           EcsCommon.h
\project        ECS
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          Common header file for the ECS

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "EcsUtils.h"
#include "Component.h"
#include "Archetype.h"
#include <algorithm>
#include  <assert.h>
namespace Ecs::internal
{
	//forward declarations
	DataChunk* Build_chunk(ComponentCombination* cmpList);
	inline int Insert_entity_in_chunk(DataChunk* chunk, EntityID EID, bool bInitializeConstructors = true);
	inline EntityID Erase_entity_in_chunk(DataChunk* chunk, uint16_t index);

	
	//retrieves a component's ComponentInfo
	template<typename T>
	static const ComponentInfo* Get_ComponentInfo() {
		static const ComponentInfo* mt = []() {
			constexpr size_t name_hash = ComponentInfo::build_hash<T>().name_hash;

			auto type = componentInfo_map.find(name_hash);
			if (type == componentInfo_map.end()) {
				constexpr ComponentInfo newtype = ComponentInfo::build<T>();
				componentInfo_map[name_hash] = newtype;
			}
			return &componentInfo_map[name_hash];
		}();
		return mt;
	}

	inline ComponentCombination* Build_component_list(const ComponentInfo** types, size_t count) {
		ComponentCombination* list = new ComponentCombination();

		//calculate total size of all components
		int component_size = sizeof(EntityID);
		for (size_t i = 0; i < count; i++) {
			component_size += types[i]->size;
		}
		//total size of allocated memory
		size_t availableStorage = sizeof(DataChunk::storage);
		//2 less than the real count to account for sizes and give some slack
		size_t itemCount = (availableStorage / component_size) - 2;

		uint32_t offsets = sizeof(DataChunkHeader);
		offsets += sizeof(EntityID) * itemCount;

		for (size_t i = 0; i < count; i++) {
			const ComponentInfo* type = types[i];

			if (type->align != 0) {
				//align properly
				size_t remainder = offsets % type->align;
				size_t oset = type->align - remainder;
				offsets += oset;
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
	//given array of component types, generates unique signature 
	//from combination of component types
	inline size_t Build_signature(const ComponentInfo** types, size_t count) {
		size_t and_hash = 0;
		//for (auto m : types)
		for (int i = 0; i < count; i++)
		{
			//consider if the fancy hash is needed, there is a big slowdown
			//size_t keyhash = ash_64_fnv1a(&types[i]->name_hash, sizeof(size_t));
			//size_t keyhash = types[i]->name_hash;

			and_hash |= types[i]->hash.matcher_hash;
			//and_hash |=(uint64_t)0x1L << (uint64_t)((types[i]->name_hash) % 63L);
		}
		return and_hash;
	}
	//add a new chunk to an archtype
	inline DataChunk* Create_chunk_for_archetype(Archetype* arch) {
		DataChunk* chunk = Build_chunk(arch->componentList);

		chunk->header.archetype = arch;
		arch->chunks.emplace_back(chunk);
		return chunk;
	}

	//comparision function for sorting components in the correct order
	inline bool Compare_ComponentInfo(const ComponentInfo* A, const ComponentInfo* B) {
		//return A->name_hash < B->name_hash;
		return A < B;
	}
	//given an array of components, sorts components in order
	inline void Sort_ComponentInfo(const ComponentInfo** types, size_t count) {
		std::sort(types, types + count, [](const ComponentInfo* A, const ComponentInfo* B) {
			return Compare_ComponentInfo(A, B);
			});
	}
	//retrieves or creates an archetype
	Archetype* Find_or_create_archetype(ECSWorld* world, const ComponentInfo** types, size_t count);
	//allocates new memory for a chunk
	inline DataChunk* Build_chunk(ComponentCombination* cmpList) {

		DataChunk* chunk = new DataChunk();
		chunk->header.last = 0;
		chunk->header.componentList = cmpList;

		return chunk;
	}
	/***********************************
	ecs world
	***********************************/

	//creates a new entity or recycles an unused one
	EntityID Allocate_entity(ECSWorld* world);

	/***********************************
	chunk
	***********************************/
	//gets the array of a component
	template<typename T>
	inline auto Get_chunk_array(DataChunk* chunk) {

		using ActualType = ::std::remove_reference_t<T>;

		if constexpr (std::is_same<ActualType, EntityID>::value)
		{
			EntityID* ptr = ((EntityID*)chunk);
			return ComponentArray<EntityID>(ptr, chunk);
		}
		else {
			constexpr TypeHash hash = ComponentInfo::build_hash<ActualType>();

			for (auto cmp : chunk->header.componentList->components) {
				if (cmp.hash == hash)
				{
					void* ptr = (void*)((byte*)chunk + cmp.chunkOffset);

					return ComponentArray<ActualType>(ptr, chunk);
				}
			}


			return ComponentArray<ActualType>();
		}
	}
	//reorder archetype with the fullness
	inline void set_chunk_full(DataChunk* chunk) {

		Archetype* arch = chunk->header.archetype;
		if (arch) {
			arch->full_chunks++;

			//do it properly later
			int archsize = arch->componentList->chunkCapacity;

			//all chunks that are full are shifted to beginning of vector
			std::partition(arch->chunks.begin(), arch->chunks.end(), 
				[archsize](DataChunk* cnk) {
				return cnk->header.last == archsize;
				});
		}


	}
	//get the most recently added chunk, or add a new one if there's none
	inline DataChunk* Find_free_chunk(Archetype* arch) {
		DataChunk* targetChunk = nullptr;
		if (arch->chunks.size() == 0) {
			targetChunk = Create_chunk_for_archetype(arch);
		}
		else {
			targetChunk = arch->chunks[arch->chunks.size() - 1];
			//chunk is full, create a new one
			if (targetChunk->header.last == arch->componentList->chunkCapacity) {
				targetChunk = Create_chunk_for_archetype(arch);
			}
		}
		return targetChunk;
	}
	//inserts an entity into a chunk
	int Insert_entity_in_chunk(DataChunk* chunk, EntityID EID, bool bInitializeConstructors);
	/***********************************
	entity
	***********************************/
	//true if entity exists and is being used
	inline bool Is_entity_valid(ECSWorld* world, EntityID id);
	//move an entity to another archtype, copying over all its components
	inline void Move_entity_to_archetype(Archetype* newarch, EntityID id, bool bInitializeConstructors = true);
	//set an entity into an archetype's entity array 
	inline void Set_entity_archetype(Archetype* arch, EntityID id);
	//create entity in an archtype
	inline EntityID Create_entity_with_archetype(Archetype* arch);
	//get the archtype an entity is in
	inline Archetype* Get_entity_archetype(ECSWorld* world, EntityID id);
	/***********************************
	component
	***********************************/
	//get an component of an entity
	template<typename C>
	C& Get_entity_component(ECSWorld* world, EntityID id)
	{

		EnityToChunk& storage = world->entities[id.index];

		auto acrray = Get_chunk_array<C>(storage.chunk);
		assert(acrray.chunkOwner != nullptr);
		return acrray[storage.chunkIndex];
	}
	//true if entity has this component
	template<typename C>
	bool Has_component(ECSWorld* world, EntityID id)
	{
		EnityToChunk& storage = world->entities[id.index];

		auto acrray = get_chunk_array<C>(storage.chunk);
		return acrray.chunkOwner != nullptr;
	}
	//adds a component to an entity via default constructor
	template<typename C>
	void Add_component_to_entity(ECSWorld* world, EntityID id)
	{
		const ComponentInfo* tempComponentInfoArray[MAX_COMPONENTS];

		const ComponentInfo* type = Get_ComponentInfo<C>();


		Archetype* oldarch = Get_entity_archetype(world, id);
		ComponentCombination* oldlist = oldarch->componentList;
		bool typeFound = false;
		int numComponents = oldlist->components.size();
		//check if component we're looking for is in this entity's archtype
		//obviously we do nothing if this entity's archtype has this component
		for (int i = 0; i < oldlist->components.size(); i++) {
			tempComponentInfoArray[i] = oldlist->components[i].type;

			//the pointers for metatypes are always fully stable
			if (tempComponentInfoArray[i] == type) {
				typeFound = true;
			}
		}

		//if this entity's archtype is missing this component 
		//we gotta find a new archtype
		Archetype* newArch = oldarch;
		if (!typeFound) {
			//sort the components into proper order
			tempComponentInfoArray[numComponents] = type;
			Sort_ComponentInfo(tempComponentInfoArray, numComponents + 1);
			numComponents++;

			//get the new archtype
			newArch = Find_or_create_archetype(world, tempComponentInfoArray, numComponents);


			//set entity into the new archtype
			Set_entity_archetype(newArch, id);
		}

	}
	//adds a component to an entity via copy
	template<typename C>
	void Add_component_to_entity(ECSWorld* world, EntityID id, C& comp)
	{
		const ComponentInfo* type = Get_ComponentInfo<C>();

		Add_component_to_entity<C>(world, id);


		//optimize later
		if (!type->is_empty()) {
			Get_entity_component<C>(world, id) = comp;
		}
	}

	/****************************
	  event functions
	****************************/




}//namespace Ecs::internal
