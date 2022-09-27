/************************************************************************************//*!
\file           Archetype.h
\project        ECS
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          Archetypes represents a unique collection of components 

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "EcsUtils.h"
#include "EventCallback.h"
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
		EventCallback addition_callbacks;
		EventCallback deletion_callbacks;
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