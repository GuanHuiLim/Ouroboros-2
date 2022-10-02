/************************************************************************************//*!
\file           World.h
\project        ECS
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          
The main object through which the entity component system functionality
is accessed.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "EcsUtils.h"
#include "Archetype.h"
#include "System.h"

#include <vector>
#include <unordered_map>
#include <queue>
namespace Ecs
{

	struct IECSWorld {
		template<typename C>
		using CompEventFnPtr = typename internal::EventFunction<ComponentEvent<C>>::FunctionPointer;
		template<typename T, typename C>
		using CompEventMemberFnPtr = typename internal::EventMemberFunction<T, ComponentEvent<C>>::MemberFunctionPointer;

		using FnPtr = typename internal::EventFunction<EntityEvent>::FunctionPointer;
		template<typename T>
		using MemberFnPtr = typename internal::EventMemberFunction<T, EntityEvent>::MemberFunctionPointer;

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

		std::unordered_map<size_t, internal::LoadedSystem> system_map{};

		int live_entities{ 0 }; //tracks number of active entity IDs
		int dead_entities{ 0 }; //tracks number of dead entity IDs

		std::queue<EntityID> addition_queue{};
		std::queue<EntityID> deletion_queue{};

		EventCallback onAddEntity_callbacks;
		EventCallback onDestroyEntity_callbacks;

		std::unordered_map<std::size_t, EventCallback> onAddComponent_Callbacks;
		std::unordered_map<std::size_t, EventCallback> onRemoveComponent_Callbacks;

		inline IECSWorld();
		~IECSWorld();

		//needs push_back(DataChunk*) to work, returns number
		template<typename Container>
		int gather_chunks(IQuery& query, Container& container);

		template<typename Func>
		void for_each(IQuery& query, Func&& function);

		template<typename Func>
		void for_each_entity(IQuery& query, Func&& function);

		template<typename Func>
		void for_each_entity_and_component(IQuery& query, Func&& function);

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
		static inline size_t get_component_hash();

		template<typename C>
		static ComponentInfo const* get_component_info();

		void* get_component(EntityID const id, size_t const hash);

		static GetCompFn* get_component_Fn(size_t const hash);



		size_t get_num_components(EntityID id);

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
		inline S* Add_System();

		template<typename S, typename... Args>
		inline S* Add_System(Args&&... arguementList);

		template<typename S>
		inline S* Get_System();

		template<typename S>
		inline void Run_System(ECSWorld* world);

		inline void SubscribeOnAddEntity(FnPtr function);
		template<typename T>
		inline void SubscribeOnAddEntity(T* instance, MemberFnPtr<T> function);

		inline void SubscribeOnDestroyEntity(FnPtr function);
		template<typename T>
		inline void SubscribeOnDestroyEntity(T* instance, MemberFnPtr<T> function);


		template<typename C>
		inline void SubscribeOnAddComponent(CompEventFnPtr<C> function);

		template<typename C>
		inline void SubscribeOnRemoveComponent(CompEventFnPtr<C> function);

		template<typename T, typename C>
		inline void SubscribeOnAddComponent(T* instance, CompEventMemberFnPtr<T,C> function);

		template<typename T, typename C>
		inline void SubscribeOnRemoveComponent(T* instance, CompEventMemberFnPtr<T, C> function);



		//void ProcessDeferredOperations();


	};
}// namespace Ecs