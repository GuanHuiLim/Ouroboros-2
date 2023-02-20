/************************************************************************************//*!
\file           EcsUtils.h
\project        ECS
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          ECS Utility header

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <cstdint>
#include <cstddef>
#include <typeinfo>
#include <utility>
#include <concepts>
namespace Ecs
{
	constexpr size_t BLOCK_MEMORY_16K = 16384;
	constexpr size_t BLOCK_MEMORY_8K = 8192;
	
	constexpr size_t MAX_COMPONENTS = 32ull;

	using byte = unsigned char;

	struct TypeHash;		
	struct ComponentInfo;	
	union  EntityID;
	struct DataChunkHeader;
	struct DataChunk;
	struct ComponentCombination; 
	struct Archetype;
	struct EnityToChunk;	
	struct IQuery;
	struct IECSWorld;
	class ECSWorld;
	class System;

	using GetCompFn = void*(IECSWorld& world, EntityID id);

	inline constexpr uint64_t hash_64_fnv1a(const char* key, const uint64_t len) {

		uint64_t hash = 14695981039346656037ull;
		uint64_t prime = 1099511628211ull;
		for (int i = 0; i < len; ++i) 
		{
			uint8_t value = key[i];
			hash = hash ^ value;
			hash *= prime;
		}
		return hash;
	}

	inline constexpr uint64_t hash_fnv1a(const char* key) {
		uint64_t hash = 14695981039346656037ull;
		uint64_t prime = 1099511628211ull;
		int i = 0;
		while (key[i]) 
		{
			uint8_t value = key[i++];
			hash = hash ^ value;
			hash *= prime;
		}
		return hash;
	}

	union EntityID 
	{
		using index_type = uint32_t;
		uint64_t value;
		struct
		{
			index_type index;
			uint32_t generation;
		};
	};

	struct TestComponent{};

	
}

namespace Ecs::internal::event
{
	struct Event
	{
	protected:
		virtual ~Event() {};
	};

}

namespace Ecs
{
	struct EntityEvent : public internal::event::Event
	{
		EntityID entity;
		EntityEvent(EntityID eid) : entity{ eid } {}
	};
}
