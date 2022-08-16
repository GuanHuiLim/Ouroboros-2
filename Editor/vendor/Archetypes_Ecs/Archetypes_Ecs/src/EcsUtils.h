#pragma once

#include <cstdint>
#include <cstddef>
#include <typeinfo>
namespace Ecs
{
	constexpr size_t BLOCK_MEMORY_16K = 16384;
	constexpr size_t BLOCK_MEMORY_8K = 8192;
	
	constexpr size_t MAX_COMPONENTS = 32ull;

	using byte = unsigned char;

	struct TypeHash;		//MetatypeHash
	struct ComponentInfo;	//Metatype
	union  EntityID;
	struct DataChunkHeader;
	struct DataChunk;
	struct ComponentCombination; //ChunkComponentList
	struct Archetype;
	struct EnityToChunk;	//EntityStorage
	struct IQuery;
	struct IECSWorld;
	struct System;

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
