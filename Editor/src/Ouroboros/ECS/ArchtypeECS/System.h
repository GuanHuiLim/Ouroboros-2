/************************************************************************************//*!
\file           System.h
\project        ECS
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          
Base system class to be inherited for convienience. 
It is OPTIONAL and not needed to work with the ECS

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
	using SystemDestructor = void(*)(void* ptr);

	struct LoadedSystem
	{
		void* system;
		SystemDestructor destructor;
	};

	class TestSystem : public System
	{
	public:
		void Run(ECSWorld* ) override {}
	};
}