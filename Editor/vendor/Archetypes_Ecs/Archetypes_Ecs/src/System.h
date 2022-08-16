#pragma once
#include "EcsUtils.h"

#include <assert.h>
#include <vector>
#include <algorithm>
namespace Ecs
{
	class System
	{
		friend struct ECSWorld;
	protected:
		//ECSWorld* world;
	public:
		System() = default;
		virtual ~System() = default;

		virtual void Run(ECSWorld* world) = 0;
	};
}