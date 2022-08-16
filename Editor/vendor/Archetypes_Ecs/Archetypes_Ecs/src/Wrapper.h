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
		template<typename Func>
		inline void for_each(IQuery& query, Func&& function)
		{
			world.for_each(query,std::forward<Func>(function));
		}

		template<typename Func>
		inline void for_each(Func&& function)
		{
			world.for_each(std::forward<Func>(function));
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

		template<typename S>
		S* Add_System()
		{
			return world.Add_System<S>();
		}

		template<typename S>
		S* Get_System()
		{
			return world.Get_System<S>();
		}
	};
}