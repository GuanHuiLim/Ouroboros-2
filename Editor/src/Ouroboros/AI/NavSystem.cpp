/************************************************************************************//*!
\file          NavSystem.cpp
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          September 19, 2022
\brief         File Contains the implementations needed for the NavSystem to function

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include <pch.h>
#include "NavSystem.h"
#include <algorithm>
#include "Ouroboros/ECS/DeferredComponent.h"

namespace oo
{
	void NavSystem::Init()
	{
		m_world->SubscribeOnAddComponent<NavSystem, NavGraphComponent>(
			this, &NavSystem::InitNavGraph);

		m_world->SubscribeOnAddComponent<NavSystem, NavAgentComponent>(
			this, &NavSystem::InitNavAgent);
	}

	void NavSystem::Run(Ecs::ECSWorld* world)
	{
		static constexpr const char* const navagent_update = "navagent_update";

		static Ecs::Query NavAgentQuery = []()
		{
		    Ecs::Query NavAgentQuery;
			NavAgentQuery.with<NavAgentComponent, TransformComponent>().exclude<DeferredComponent>().build();
		    return NavAgentQuery;
		}();
		world->for_each(NavAgentQuery, [&](NavAgentComponent& agent, TransformComponent& tf){ Pathfind(agent, tf, agent.target); });

		static Ecs::Query NavGraphQuery = []()
		{
			Ecs::Query NavGraphQuery;
			NavGraphQuery.with<NavGraphComponent, TransformComponent>().exclude<DeferredComponent>().build();
			return NavGraphQuery;
		}();
		world->for_each(NavGraphQuery, [&](NavGraphComponent& graph, TransformComponent& tf) {  });
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///NavGraph Function Codes                                                                                         ///
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void NavSystem::InitNavGraph(Ecs::ComponentEvent<NavGraphComponent>* evnt)
	{
		evnt->component.nodeDiameter = evnt->component.nodeRadius * 2;
		evnt->component.gridSizeX = evnt->component.gridWorldSize.x / evnt->component.nodeDiameter;
		evnt->component.gridSizeY = evnt->component.gridWorldSize.y / evnt->component.nodeDiameter;
		CreateGrid();
	}

	void NavSystem::CreateGrid()
	{


	}

	std::list<Node*> NavSystem::GetNeighbours(NavGraphComponent& navGraph, Node* node)
	{
		std::list<Node*> neighbours;

		for (int x = -1; x <= 1; ++x)
		{
			for (int y = -1; y <= 1; ++y)
			{
				if (x == 0 && y == 0)
					continue;

				int checkX = node->gridX + x;
				int checkY = node->gridY + y;

				if (checkX >= 0 && checkX < navGraph.gridSizeX
					&& checkY >= 0 && checkY < navGraph.gridSizeY)
				{
					neighbours.push_front(navGraph.grid[checkX][checkY]);
				}
			}
		}

		return neighbours;
	}

	Node* NavSystem::NodeFromWorldPoint(NavGraphComponent& navGraph, Vector3* node)
	{
		float percentX = (node->x + navGraph.gridWorldSize.x / 2) / navGraph.gridWorldSize.x;
		float percentY = (node->y + navGraph.gridWorldSize.y / 2) / navGraph.gridWorldSize.y;
		percentX = std::clamp(percentX, 0.0f, 1.0f);
		percentY = std::clamp(percentY, 0.0f, 1.0f);

		int x = static_cast<int>(std::round((navGraph.gridSizeX - 1) * percentX));
		int y = static_cast<int>(std::round((navGraph.gridSizeY - 1) * percentY));

		return navGraph.grid[x][y];
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///NavAgent Function Codes                                                                                         ///
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void NavSystem::InitNavAgent(Ecs::ComponentEvent<NavAgentComponent>* evnt)
	{
		evnt->component.currentSpeed = evnt->component.speed;
	}

	void NavSystem::Pathfind(NavAgentComponent& agent, TransformComponent& tf, Vector3* target)
	{
		Vector3 start = tf.GetGlobalPosition();
		FindPath(agent, &start, target);
		FollowPath(agent, tf);
	}

	void NavSystem::FindPath(NavAgentComponent& agent, Vector3* start, Vector3* end)
	{
		//if (!start || !end)
		//	return;

		//bool pathSuccess = false;

		//Node* startNode = NodeFromWorldPoint(agent.grid, start);
		//Node* endNode = NodeFromWorldPoint(agent.grid, end);

		//if (!startNode->walkable)
		//{
		//	std::list<Node*> neighbours = GetNeighbours(agent.grid, startNode);
		//	for (Node* n : neighbours)
		//	{
		//		if (!n->walkable)
		//		{
		//			startNode = n;
		//			break;
		//		}
		//	}
		//}

		//if (startNode->walkable && endNode->walkable)
		//{
		//	std::list<Node*> openSet = {};
		//	std::unordered_set<Node*> closedSet = {};

		//	openSet.push_front(startNode);

		//	while (openSet.size() > 0)
		//	{
		//		Node* node = openSet.front();
		//		openSet.pop_front();

		//		closedSet.insert(node);

		//		if (node == endNode)
		//		{
		//			pathSuccess = true;
		//			break;
		//		}

		//		for (Node* neighbour : GetNeighbours(agent.grid, node))
		//		{
		//			if (!neighbour->walkable || closedSet.contains(neighbour))
		//			{
		//				continue;
		//			}

		//			int newCostToNeighbour = node->gCost + GetDistance(node, neighbour);
		//			bool openSetContainsNeighbour = (std::find(openSet.begin(), openSet.end(), neighbour) != openSet.end());
		//			if (newCostToNeighbour < neighbour->gCost || !openSetContainsNeighbour)
		//			{
		//				neighbour->gCost = newCostToNeighbour;
		//				neighbour->hCost = GetDistance(neighbour, endNode);
		//				neighbour->parent = node;

		//				if (!openSetContainsNeighbour)
		//				{
		//					openSet.push_front(neighbour);
		//				}

		//			}
		//		}
		//	}
		//}

		//if (pathSuccess)
		//{
		//	agent.currPath = RetracePath(startNode, endNode);
		//}
	}

	std::vector<NavSystem::Vector3*> NavSystem::RetracePath(Node* start, Node* end)
	{
		std::vector<Node*> path = {};
		Node* currentNode = end;

		while (currentNode != start)
		{
			path.insert(path.begin(), currentNode);
			currentNode = currentNode->parent;
		}

		std::vector<Vector3*> waypoints = SimplifyPath(path);

		std::reverse(waypoints.begin(), waypoints.end());

		return waypoints;
	}

	std::vector<NavSystem::Vector3*> NavSystem::SimplifyPath(std::vector<Node*> path)
	{
		std::vector<Vector3*> waypoints = {};
		Vector2 directionOld = Vector2{ 0.0f,0.0f };

		for (int i = 0; i < path.size(); ++i)
		{
			Vector2 directionNew = Vector2{path[i - 1]->gridX - path[i]->gridX,
										   path[i - 1]->gridX - path[i]->gridX};

			if (directionNew != directionOld)
			{
				Vector3 temp = path[i]->worldPosition + Vector3{ 0, 1, 0 };
				waypoints.insert(waypoints.begin(), &temp);
			}
			directionNew = directionOld;
		}

		return waypoints;
	}

	void NavSystem::FollowPath(NavAgentComponent& agent, TransformComponent& tf)
	{
		if (agent.currPath.size() == 0)
		{
			agent.reached = true;
			return;
		}

		agent.reached = false;
		agent.currentSpeed = agent.speed;
		Vector3 currentWayPoint = Vector3{ agent.currPath[0]->x,
										   agent.currPath[0]->y + tf.GetScale().y,
										   agent.currPath[0]->z };

		while (true)
		{
			if (tf.Position() == currentWayPoint)
			{
				if (agent.targetIndex < agent.currPath.size())
					agent.targetIndex++;
				else
					break;
				Vector3 currentWayPoint = Vector3{ agent.currPath[agent.targetIndex]->x,
										           agent.currPath[agent.targetIndex]->y + tf.GetScale().y,
										           agent.currPath[agent.targetIndex]->z };
			}

			//move the agent
			tf.LookAt(Vector3{currentWayPoint.x, tf.GetGlobalPosition().y,  currentWayPoint.z });
			agent.direction = currentWayPoint - tf.GetGlobalPosition();
			//translate the go

			return;
		}
	}

	int NavSystem::GetDistance(Node* nodeA, Node* nodeB)
	{
		int dstX = abs(nodeA->gridX - nodeB->gridX);
		int dstY = abs(nodeA->gridY - nodeB->gridY);

		if(dstX > dstY)
			return 14 * dstY + 10 * (dstX - dstY);
		else
			return 14 * dstX + 10 * (dstY - dstX);
	}
}
