/************************************************************************************//*!
\file           Query.h
\project        ECS
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          
Internal base query class that allows for setting filters for the ECS's iterative
functionalities

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "EcsUtils.h"
#include "Component.h"

#include <assert.h>
#include <vector>
#include <algorithm>
namespace Ecs
{
	struct IQuery {
		std::vector<TypeHash> require_comps;
		std::vector<TypeHash> exclude_comps;


		size_t require_matcher{ 0 };
		size_t exclude_matcher{ 0 };


		bool built{ false };

		template<typename... C>
		IQuery& with() {
			require_comps.insert(require_comps.end(), { ComponentInfo::build_hash<C>()... });

			return *this;
		}

		template<typename... C>
		IQuery& exclude() {
			exclude_comps.insert(exclude_comps.end(), { ComponentInfo::build_hash<C>()... });

			return *this;
		}

		IQuery& build() {
			auto compare_hash = [](const TypeHash& A, const TypeHash& B) {
				return A.name_hash < B.name_hash;
			};

			auto build_matcher = [](const std::vector<TypeHash>& types) {
				size_t and_hash = 0;

				for (auto type : types)
				{
					and_hash |= type.matcher_hash;
				}
				return and_hash;
			};

			auto remove_eid = [](const TypeHash& type) {
				return (type == ComponentInfo::build_hash<EntityID>());
			};
			require_comps.erase(std::remove_if(require_comps.begin(), require_comps.end(), remove_eid), require_comps.end());
			exclude_comps.erase(std::remove_if(exclude_comps.begin(), exclude_comps.end(), remove_eid), exclude_comps.end());

			std::sort(require_comps.begin(), require_comps.end(), compare_hash);
			std::sort(exclude_comps.begin(), exclude_comps.end(), compare_hash);

			require_matcher = build_matcher(require_comps);
			exclude_matcher = build_matcher(exclude_comps);
			built = true;
			return *this;
		}
	};
}