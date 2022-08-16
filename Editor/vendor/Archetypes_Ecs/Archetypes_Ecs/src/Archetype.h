#pragma once
#include "EcsUtils.h"

#include <assert.h>

namespace Ecs
{
	//represents a unique collection of components
	struct Archetype {
		ComponentCombination* componentList{nullptr};
		struct IECSWorld* ownerWorld{ nullptr };
		size_t componentHash{0ul}; //archetype signature computed from all the components
		int full_chunks{0};
		//full chunks allways on the start of the array
		std::vector<DataChunk*> chunks{};
	};

	//contains info for mapping which chunk an entity belongs to
	struct EnityToChunk
	{
		DataChunk* chunk;
		uint32_t generation;
		uint16_t chunkIndex;

		bool operator==(const EnityToChunk& other) const {
			return chunk == other.chunk && generation == other.generation && chunkIndex == other.chunkIndex;
		}

		bool operator!=(const EnityToChunk& other) const {
			return !(other == *this);
		}
	};


	
}