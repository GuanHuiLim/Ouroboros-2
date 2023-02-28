#include "pch.h"
#include "WaypointSystem.h"

#include "Ouroboros/Transform/TransformComponent.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/Core/Timer.h"

#include <glm/ext/matrix_transform.hpp>

namespace oo
{
    oo::WaypointSystem::WaypointSystem(Scene* scene)
        : m_scene {scene}
    {
        EventManager::Subscribe<WaypointSystem, GameObjectComponent::OnEnableEvent>(this, &WaypointSystem::OnEnableGameObject);
        EventManager::Subscribe<WaypointSystem, GameObjectComponent::OnDisableEvent>(this, &WaypointSystem::OnDisableGameObject);
    }

    oo::WaypointSystem::~WaypointSystem()
    {
        EventManager::Unsubscribe<WaypointSystem, GameObjectComponent::OnEnableEvent>(this, &WaypointSystem::OnEnableGameObject);
        EventManager::Unsubscribe<WaypointSystem, GameObjectComponent::OnDisableEvent>(this, &WaypointSystem::OnDisableGameObject);
    }

    void oo::WaypointSystem::PostLoadSceneInit()
    {
        // initialize waypoint set data
        static Ecs::Query waypoint_set_query = Ecs::make_raw_query<WaypointSetComponent, TransformComponent, GameObjectComponent>();
        m_world->for_each(waypoint_set_query, [&](WaypointSetComponent& waypointSetComp, TransformComponent& transformComp, GameObjectComponent& goc)
            {
                //SetUpWaypointSet(waypointSetComp);
                
                // Reset Waypoint set component values
                waypointSetComp.TotalTimeElapsed = 0.f;

                // Get All child under it that has components WaypointNodeComponent
                // Link up the nodes properly within the set.
                auto&& go = m_scene->FindWithInstanceID(goc.Id);
                auto&& childs = go->GetDirectChilds();

                // check if there's no childs
                if (childs.size() == 0)
                    return;

                WaypointNodeComponent* previous = nullptr;
                for (auto&& child : childs)
                {
                    if (child.HasComponent<WaypointNodeComponent>() == false)
                        continue;

                    // take this opportunity to reset waypoint node values as well
                    WaypointNodeComponent& current = child.GetComponent<WaypointNodeComponent>();
                    current.WaitedFor = 0.f;
                    current.MovedFor = 0.f;

                    if (previous)
                    {
                        previous->NextNodeUUID = child.GetInstanceID();
                    }
                    previous = &current;
                }

                // set waypointSet to point to the first item
                 waypointSetComp.FirstNodeUUID = waypointSetComp.CurrentNodeUUID = childs.front().GetInstanceID();
                

            });
    }

    void oo::WaypointSystem::EditorUpdate()
    {
        UpdateJustCreated();
        UpdateDuplicated();
        UpdateExisting();
    }

    void oo::WaypointSystem::RuntimeUpdate()
    {
        UpdateJustCreated();
        UpdateDuplicated();
        UpdateExisting();
    }

    void oo::WaypointSystem::UpdateJustCreated()
    {
    }

    void oo::WaypointSystem::UpdateDuplicated()
    {
    }

    void oo::WaypointSystem::UpdateExisting()
    {
        // initialize waypoint set data
        static Ecs::Query waypoint_set_query = Ecs::make_query<WaypointSetComponent, TransformComponent, GameObjectComponent>();
        m_world->for_each(waypoint_set_query, [&](WaypointSetComponent& waypointSetComp, TransformComponent& transformComp, GameObjectComponent& goc)
            {
                // we skip if we can't find an object we are interested in
                Scene::go_ptr targetGo = nullptr; // m_scene->FindWithName(waypointSetComp.TargetName);
                if (targetGo == nullptr)
                    return;

                // update waypoint
                auto dt = timer::dt();
                Scene::go_ptr currNodeGo = m_scene->FindWithInstanceID(waypointSetComp.CurrentNodeUUID);
                if (currNodeGo == nullptr)  // we've reached the end
                    return;

                auto& curNode = currNodeGo->GetComponent<WaypointNodeComponent>();

                // we go skip as well if we are at the last node.
                Scene::go_ptr nextNodeGo = m_scene->FindWithInstanceID(curNode.NextNodeUUID);
                if (nextNodeGo == nullptr)
                    return;

                // start variables
                glm::vec3 curPos, curScale, nextPos, nextScale;
                glm::quat curRot, nextRot;

                glm::mat4 finalMat;

                float barycentric_ratio = 0.f;

                auto& curTf = currNodeGo->Transform();
                curPos = curTf.GetGlobalPosition();
                curRot = curTf.GetGlobalRotationQuat();
                curScale = curTf.GetGlobalScale();

                // update currNode
                if (curNode.WaitedFor < curNode.WaitTimeBeforeMoving)
                {
                    curNode.WaitedFor += dt;
                }
                else if (curNode.MovedFor < curNode.TimeToNextNode)
                {
                    curNode.MovedFor += dt;
                }
                else // we go to the next node
                {
                    // perhaps reset curr node value too?

                    waypointSetComp.CurrentNodeUUID = curNode.NextNodeUUID;
                }

                auto& nextTf = nextNodeGo->Transform();
                nextPos = nextTf.GetGlobalPosition();
                nextRot = nextTf.GetGlobalRotationQuat();
                nextScale = nextTf.GetGlobalScale();

                // update ratio to interpolate by
                barycentric_ratio = curNode.MovedFor / curNode.TimeToNextNode;

                // update total elapsed time
                waypointSetComp.TotalTimeElapsed += dt;

                // calculate final position depending on update scheme of current node

                glm::mat4 finalT = glm::translate(glm::mat4{ 1.f }, glm::lerp(curPos, nextPos, barycentric_ratio));
                glm::quat finalQ = glm::lerp(curRot, nextRot, barycentric_ratio);
                glm::mat4 finalR = glm::mat4_cast(finalQ);
                glm::mat4 finalS = glm::scale(glm::mat4{ 1.f }, glm::lerp(curScale, nextScale, barycentric_ratio));
                finalMat = finalT * finalR * finalS;
                
                // Update position of the gameobject we're interested in
                targetGo->Transform().SetGlobalTransform(finalMat);
            });
    }



    void oo::WaypointSystem::SetUpWaypointSet()
    {
    }

    void oo::WaypointSystem::UpdateWaypointNode()
    {
    }



    void oo::WaypointSystem::OnEnableGameObject(GameObjectComponent::OnEnableEvent* e)
    {
    }

    void oo::WaypointSystem::OnDisableGameObject(GameObjectComponent::OnDisableEvent* e)
    {
    }

}
