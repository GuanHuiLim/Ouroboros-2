#pragma once
#include "Query.h"
#include "World.h"
namespace Ecs
{
	class Query
	{
		IQuery query;
	public:
		template<typename... C>
		Query& with() {
			query.with<C...>();
			return *this;
		}

		template<typename... C>
		Query& exclude() {
			query.exclude<C...>();
			return *this;
		}

		Query& build() {
			query.build();
			return *this;
		}

		operator IQuery&() { return query; }
	};



	class ECSWorld
	{
		IECSWorld world;
	public:
		template<typename C>
		using CompEventFnPtr = typename IECSWorld::CompEventFnPtr<C>;
		template<typename T, typename C>
		using CompEventMemberFnPtr = typename IECSWorld::CompEventMemberFnPtr<T,C>;


		using FnPtr = typename IECSWorld::FnPtr;
		template<typename T>
		using MemberFnPtr = typename IECSWorld::MemberFnPtr<T>;


		//supports functions of the format: exampleFunction(ComponentA&, ComponentB&,...)
		template<typename Func>
		inline void for_each(IQuery& query, Func&& function)
		{
			world.for_each(query,std::forward<Func>(function));
		}
		//supports functions of the format: exampleFunction()
		template<typename Func>
		inline void for_each(Func&& function)
		{
			world.for_each(std::forward<Func>(function));
		}
		//supports functions of the format: exampleFunction(Ecs::EntityID)
		template<typename Func>
		inline void for_each_entity(IQuery& query, Func&& function)
		{
			world.for_each_entity(query, std::forward<Func>(function));
		}
		//supports functions of the format: exampleFunction(Ecs::EntityID, ComponentA&, ComponentB&,...)
		template<typename Func>
		inline void for_each_entity_and_component(IQuery& query, Func&& function)
		{
			world.for_each_entity_and_component(query, std::forward<Func>(function));
		}

		template<typename C>
		void add_component(EntityID id, C& comp)
		{
			world.add_component<C>(id,comp);
		}
		template<typename C>
		void add_component(EntityID id)
		{
			world.add_component<C>(id);
		}
		//priority
		template<typename C>
		void remove_component(EntityID id)
		{
			world.remove_component<C>(id);
		}
		//priority
		template<typename C>
		bool has_component(EntityID id)
		{
			return world.has_component<C>(id);
		}
		//priority
		template<typename C>
		C& get_component(EntityID id)
		{
			return world.get_component<C>(id);
		}

		size_t get_num_components(EntityID id)
		{
			return world.get_num_components(id);
		}

		template<typename C>
		C* set_singleton()
		{
			return world.set_singleton<C>();
		}
		template<typename C>
		C* set_singleton(C&& singleton)
		{
			return world.set_singleton<C>(singleton);
		}

		template<typename C>
		C* get_singleton()
		{
			return world.get_singleton<C>();
		}

		template<typename ... Comps>
		inline EntityID new_entity()
		{
			return world.new_entity<Comps...>();
		}

		EntityID new_entity(std::vector<uint64_t>const& component_hashes);
		EntityID duplicate_entity(EntityID id);

		std::vector<uint64_t> const componentHashes(EntityID id);

		void destroy(EntityID eid);
		
		/*template<typename S>
		S* Add_System()
		{
			return world.Add_System<S>();
		}*/

		//adds a system, constructor arguements supported
		template<typename S, typename... Args>
		S* Add_System(Args&&... arguementList)
		{
			return world.Add_System<S, Args...>(std::forward<Args>(arguementList)...);
		}

		template<typename S>
		S* Get_System()
		{
			return world.Get_System<S>();
		}

		template<typename S>
		void Run_System()
		{
			world.Run_System<S>(this);
		}

		//requires type Ecs::EntityEvent* for the function's parameters
		void SubscribeOnAddEntity(FnPtr function);
		//requires type Ecs::EntityEvent* for the function's parameters
		//T is member function's class
		template<typename T>
		void SubscribeOnAddEntity(T* instance, MemberFnPtr<T> function)
		{
			world.SubscribeOnAddEntity<T>(instance, function);
		}

		//requires type Ecs::EntityEvent* for the function's parameters
		void SubscribeOnDestroyEntity(FnPtr function);
		//requires type Ecs::EntityEvent* for the function's parameters
		//T is member function's class
		template<typename T>
		void SubscribeOnDestroyEntity(T* instance, MemberFnPtr<T> function)
		{
			world.SubscribeOnDestroyEntity(instance, function);
		}
		//C is Component type
		template<typename C>
		void SubscribeOnAddComponent(CompEventFnPtr<C> function)
		{
			world.SubscribeOnAddComponent<C>(function);
		}
		//C is Component type
		template<typename C>
		void SubscribeOnRemoveComponent(CompEventFnPtr<C> function)
		{
			world.SubscribeOnRemoveComponent<C>(function);
		}

		//T is the type of the member function's class, C is Component type
		template<typename T, typename C>
		void SubscribeOnAddComponent(T* instance, CompEventMemberFnPtr<T, C> function)
		{
			world.SubscribeOnAddComponent<T,C>(instance, function);
		}
		//T is the type of the member function's class, C is Component type
		template<typename T, typename C>
		void SubscribeOnRemoveComponent(T* instance, CompEventMemberFnPtr<T, C> function)
		{
			world.SubscribeOnRemoveComponent<T, C>(instance, function);
		}

	};
}