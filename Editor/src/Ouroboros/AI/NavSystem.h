/************************************************************************************//*!
\file          NavSystem.h
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 13, 2022
\brief         File Contains the declaration for NavSystem

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <Ouroboros/Scene/Scene.h>
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Transform/TransformComponent.h>

#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>

#include <vector>

#include "Node.h"
#include "NavGraphComponent.h"
#include "NavAgentComponent.h"

namespace oo
{
	class NavSystem final: public Ecs::System
	{
		using Vector3 = glm::vec3;
		using Vector2 = glm::vec2;
	public:
		NavSystem(Scene* scene) : m_scene{scene} {}
		virtual ~NavSystem() = default;

		virtual void Run(Ecs::ECSWorld* world) override;

		void Init();

	private:

		//NavGraphMethods
		void InitNavGraph(Ecs::ComponentEvent<NavGraphComponent>* evnt);
		void CreateGrid();
		std::list<Node*> GetNeighbours(NavGraphComponent& navGraph, Node* node);
		Node* NodeFromWorldPoint(NavGraphComponent& navGraph, Vector3* node);

		//NavAgentMethods
		void InitNavAgent(Ecs::ComponentEvent<NavAgentComponent>* evnt);
		void Pathfind(NavAgentComponent& agent, TransformComponent& tf, Vector3* target);
		void FindPath(NavAgentComponent& agent, Vector3* start, Vector3* end);
		std::vector<Vector3*> RetracePath(Node* start, Node* end);
		std::vector<Vector3*> SimplifyPath(std::vector<Node*> path);
		void FollowPath(NavAgentComponent& agent, TransformComponent& tf);
		int GetDistance(Node* nodeA, Node* nodeB);

		Scene* m_scene;
		std::vector<NavGraphComponent*> graphs;
	};
}
