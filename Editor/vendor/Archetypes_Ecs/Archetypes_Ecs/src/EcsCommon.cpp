/************************************************************************************//*!
\file           EcsCommon.cpp
\project        ECS
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          see header file 

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "EcsCommon.h"
#include "World.h"
namespace Ecs::internal
{

	Archetype* Find_or_create_archetype(ECSWorld* world, const ComponentInfo** types, size_t count) {
		const ComponentInfo* temporalComponentInfoArray[MAX_COMPONENTS];
		assert(count < MAX_COMPONENTS);

		const ComponentInfo** typelist;

		if (false) {//!is_sorted(types, count)) {
			for (int i = 0; i < count; i++) {
				temporalComponentInfoArray[i] = types[i];

			}
			Sort_ComponentInfo(temporalComponentInfoArray, count);
			typelist = temporalComponentInfoArray;
		}
		else {
			typelist = types;
		}

		const uint64_t matcher = Build_signature(typelist, count);

		//try to find the archetype in the hashmap
		auto iter = world->archetype_signature_map.find(matcher);
		if (iter != world->archetype_signature_map.end()) {

			//iterate the vector of archetypes
			auto& archvec = iter->second;//world->archetype_signature_map[matcher];
			for (int i = 0; i < archvec.size(); i++) {

				auto componentList = archvec[i]->componentList;
				int ccount = componentList->components.size();
				// if requested number of components matches archetype's
				// number of components
				if (ccount == count) {
					for (int j = 0; j < ccount; j++) {
						//check if components matches the typelist
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
		newArch->componentList = Build_component_list(typelist, count);
		newArch->componentHash = matcher;
		newArch->ownerWorld = world;
		world->archetypes.push_back(newArch);
		world->archetypeSignatures.push_back(matcher);
		world->archetype_signature_map[matcher].push_back(newArch);

		//we want archs to allways have 1 chunk at least, create initial
		Create_chunk_for_archetype(newArch);
		return newArch;
	}

	//creates a new entity or recycles an unused one
	EntityID Allocate_entity(ECSWorld* world) {
		EntityID newID;
		//no dead entity IDs
		if (world->dead_entities == 0) {
			int index = world->entities.size();

			EnityToChunk entity_chunk_mapping;
			entity_chunk_mapping.chunk = nullptr;
			entity_chunk_mapping.chunkIndex = 0;
			entity_chunk_mapping.generation = 1;

			world->entities.push_back(entity_chunk_mapping);

			newID.generation = 1;
			newID.index = index;
		}
		else { //dead entity IDs available for reuse
			int index = world->deletedEntities.back();
			world->deletedEntities.pop_back();

			//increment generation count as we are recycling it
			world->entities[index].generation++;
			//update our return value
			newID.generation = world->entities[index].generation;
			newID.index = index;
			//decrease number of dead entity IDs available
			world->dead_entities--;
		}

		world->live_entities++;
		return newID;
	}

	//inserts an entity into a chunk
	int Insert_entity_in_chunk(DataChunk* chunk, EntityID EID, bool bInitializeConstructors) {
		int index = -1;

		ComponentCombination* cmpList = chunk->header.componentList;

		//if chunk has not reached maximum capacity
		if (chunk->header.last < cmpList->chunkCapacity) {

			index = chunk->header.last;
			chunk->header.last++;

			if (bInitializeConstructors) {
				//initialize component
				for (auto& cmp : cmpList->components) {
					const ComponentInfo* mtype = cmp.type;

					//if is a component with data
					if (!mtype->is_empty()) {
						void* ptr = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * index));

						mtype->constructor(ptr);
					}
				}
			}


			//set eid at data chunk
			EntityID* eidptr = ((EntityID*)chunk);
			eidptr[index] = EID;

			//if full, reorder it on archetype
			if (chunk->header.last == cmpList->chunkCapacity) {
				set_chunk_full(chunk);
			}
		}

		return index;
	}

	//true if entity exists and is being used
	bool Is_entity_valid(ECSWorld* world, EntityID id) {
		//index check
		if (world->entities.size() > id.index && id.index >= 0) {

			//generation check
			if (id.generation != 0
				&& world->entities[id.index].generation == id.generation)
			{
				return true;
			}
		}
		return false;
	}
	//move an entity to another archtype, copying over all its components
	void Move_entity_to_archetype(Archetype* newarch, EntityID id, bool bInitializeConstructors) {

		//insert into new chunk
		DataChunk* oldChunk = newarch->ownerWorld->entities[id.index].chunk;
		DataChunk* newChunk = Find_free_chunk(newarch);

		int newindex = Insert_entity_in_chunk(newChunk, id, bInitializeConstructors);
		int oldindex = newarch->ownerWorld->entities[id.index].chunkIndex;

		int oldNcomps = oldChunk->header.componentList->components.size();
		int newNcomps = newChunk->header.componentList->components.size();

		auto& oldClist = oldChunk->header.componentList;
		auto& newClist = newChunk->header.componentList;

		//copy all data from old chunk into new chunk
		//bad iteration, fix later

		struct Merge {
			int msize;
			int idxOld;
			int idxNew;
		};
		int mergcount = 0;
		Merge mergarray[MAX_COMPONENTS];

		for (int i = 0; i < oldNcomps; i++) {
			const ComponentInfo* mtCp1 = oldClist->components[i].type;
			if (!mtCp1->is_empty()) {
				for (int j = 0; j < newNcomps; j++) {
					const ComponentInfo* mtCp2 = newClist->components[j].type;

					//pointers are stable
					if (mtCp2 == mtCp1) {
						mergarray[mergcount].idxNew = j;
						mergarray[mergcount].idxOld = i;
						mergarray[mergcount].msize = mtCp1->size;
						mergcount++;
					}
				}
			}
		}

		for (int i = 0; i < mergcount; i++) {
			//const Metatype* mtCp1 = mergarray[i].mtype;

			//pointer for old location in old chunk
			void* ptrOld = (void*)((byte*)oldChunk + oldClist->components[mergarray[i].idxOld].chunkOffset + (mergarray[i].msize * oldindex));

			//pointer for new location in new chunk
			void* ptrNew = (void*)((byte*)newChunk + newClist->components[mergarray[i].idxNew].chunkOffset + (mergarray[i].msize * newindex));

			//memcopy component data from old to new
			memcpy(ptrNew, ptrOld, mergarray[i].msize);
		}

		//delete entity from old chunk
		Erase_entity_in_chunk(oldChunk, oldindex);

		//assign entity chunk data
		newarch->ownerWorld->entities[id.index].chunk = newChunk;
		newarch->ownerWorld->entities[id.index].chunkIndex = newindex;
	}

	//set an entity into an archetype's entity array 
	void Set_entity_archetype(Archetype* arch, EntityID id) {

		//if chunk is null, we are a empty entity
		if (arch->ownerWorld->entities[id.index].chunk == nullptr) {

			DataChunk* targetChunk = Find_free_chunk(arch);

			int index = Insert_entity_in_chunk(targetChunk, id);
			arch->ownerWorld->entities[id.index].chunk = targetChunk;
			arch->ownerWorld->entities[id.index].chunkIndex = index;
		}
		else {
			Move_entity_to_archetype(arch, id, false);
		}
	}

	//create entity in an archtype
	EntityID Create_entity_with_archetype(Archetype* arch) {
		ECSWorld* world = arch->ownerWorld;

		EntityID newID = Allocate_entity(world);

		Set_entity_archetype(arch, newID);

		return newID;
	}

	//get the archtype an entity is in
	Archetype* Get_entity_archetype(ECSWorld* world, EntityID id)
	{
		assert(Is_entity_valid(world, id));

		return world->entities[id.index].chunk->header.archetype;
	}

}

