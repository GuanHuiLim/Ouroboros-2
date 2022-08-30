#pragma once
#include "EcsUtils.h"

#include <assert.h>
#include <vector>
#include <algorithm>
namespace Ecs
{
	class System
	{
		friend struct IECSWorld;
	protected:
		IECSWorld* m_world;
	public:
		System() = default;
		virtual ~System() = default;

		virtual void Run(ECSWorld* world) = 0;
	};
}

namespace Ecs::internal
{
	class TestSystem : public System
	{
	public:
		void Run(ECSWorld* ) override {}
	};
}