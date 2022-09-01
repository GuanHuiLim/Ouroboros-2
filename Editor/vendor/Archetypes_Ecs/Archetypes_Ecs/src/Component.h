#pragma once
#include "EcsUtils.h"

#include <typeinfo>
#include <vector>
#include <unordered_map>

namespace Ecs
{
	extern std::unordered_map<uint64_t, ComponentInfo> componentInfo_map; //metatype_cache

	struct TypeHash
	{
		std::size_t name_hash{ 0 };
		std::size_t matcher_hash{ 0 };

		bool operator==(const TypeHash& other)const {
			return name_hash == other.name_hash;
		}
		template<typename T>
		static constexpr const char* name_detail() {
			return __FUNCSIG__;
		}

		template<typename T>
		static constexpr size_t hash() {

			static_assert(!std::is_reference_v<T>, "dont send references to hash");
			static_assert(!std::is_const_v<T>, "dont send const to hash");
			return hash_fnv1a(name_detail<T>());
		}
	};
	//struct containing information about a unique component type
	struct ComponentInfo
	{
		using ConstructorFn = void(void*);
		using DestructorFn = void(void*);
		using CopyConstructorFn = void(void* self, void* copy);
		using MoveConstructorFn = void(void* self, void* other);
		using MoveAssignmentFn = void(void* self, void* other);

		TypeHash hash{};

		const char* name{ "none" }; //name 
		ConstructorFn* constructor{nullptr};
		DestructorFn* destructor{ nullptr };
		CopyConstructorFn* copy_constructor{ nullptr };
		MoveConstructorFn* move_constructor{ nullptr };
		MoveAssignmentFn*  move_assignment{ nullptr };
		uint16_t size{ 0 };
		uint16_t align{ 0 };

		bool is_empty() const { return align == 0u; };

		template<typename T>
		static constexpr TypeHash build_hash() {

			using base_type = std::remove_const_t<std::remove_reference_t<T>>;

			TypeHash hash;
			hash.name_hash = TypeHash::hash<base_type>();
			hash.matcher_hash |= (uint64_t)0x1L << (uint64_t)((hash.name_hash) % 63L);
			return hash;
		};
		template<typename T>
		static constexpr ComponentInfo build() {

			ComponentInfo info{};
			info.hash = build_hash<T>();
			info.name = typeid(T).name();

			if constexpr (std::is_empty_v<T>)
			{
				info.align = 0;
				info.size = 0;
			}
			else {
				info.align = alignof(T);
				info.size = sizeof(T);
			}

			info.constructor = [](void* p)
			{
				new(p) T{};
			};
			info.destructor = [](void* p)
			{
				((T*)p)->~T();
			};

			info.copy_constructor = [](void* self, void* copy)
			{
				new(self) T{*(static_cast<T*>(copy))};
			};

			info.move_constructor = [](void* self, void* other)
			{
				new(self) T{ std::move( *(static_cast<T*>(other)) ) };
			};

			info.move_assignment = [](void* self, void* other)
			{
				T* ptr = static_cast<T*>(self);
				*ptr = std::move(*(static_cast<T*>(other)));
			};

			return info;
		};
	};

	//linked list header
	struct DataChunkHeader {
		struct ComponentCombination* componentList{nullptr}; //pointer to the signature for this block
		struct Archetype* archetype{ nullptr };	//what archtype this data chunk contains
		struct DataChunk* prev{ nullptr };
		struct DataChunk* next{ nullptr };
		int16_t last{ 0 }; //one after the last entity added
	};
	// header | entityID | component 1 data | component 2 data |...
	struct alignas(32)DataChunk 
	{
		std::byte storage[BLOCK_MEMORY_16K - sizeof(DataChunkHeader)]{};
		DataChunkHeader header{};
	};
	static_assert(sizeof(DataChunk) == BLOCK_MEMORY_16K, "chunk size isnt 16kb");

	//set of unique combination of components
	struct ComponentCombination {
		struct ComponentIdentifier {
			const ComponentInfo* type; //information about the type
			TypeHash hash; // hash of the component
			uint32_t chunkOffset; //offset from start of chunk
		};
		int16_t chunkCapacity{};
		std::vector<ComponentIdentifier> components{}; //all the components in this archtype
	};

	//an array for storing components
	template<typename T>
	struct ComponentArray 
	{
		T* data{ nullptr };
		DataChunk* chunkOwner{ nullptr };

		ComponentArray() = default;
		ComponentArray(void* pointer, DataChunk* owner) {
			data = (T*)pointer;
			chunkOwner = owner;
		}

		const T& operator[](size_t index) const {
			return data[index];
		}
		T& operator[](size_t index) {
			return data[index];
		}
		bool valid()const {
			return data != nullptr;
		}
		T* begin() {
			return data;
		}
		T* end() {
			return data + chunkOwner->header.last;
		}
		int16_t size() {
			return chunkOwner->header.last;
		}
	};


	template<typename T>
	struct ComponentEvent : public Ecs::internal::event::Event
	{
		T& component;
		EntityID entityID;
		ComponentEvent(EntityID eid, T& _component) 
			: component{ _component }, entityID{ eid } {};
	};

	using TestComponentEvent = ComponentEvent<TestComponent>;
}
