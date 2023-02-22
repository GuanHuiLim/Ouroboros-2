/************************************************************************************//*!
\file           Scene.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 31, 2022
\brief          Scene defines the basis of what makes a scene and most of its core
                features such as tracking all the gameobjects, having access to the 
                scene graph and root gameobject.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <SceneManagement/include/IScene.h>
#include "Ouroboros/EventSystem/Event.h"

#include <scenegraph/include/scenegraph.h>
#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
#include <set>
#include <functional>
#include "Utility/UUID.h"

#include <OO_Vulkan/src/GraphicsWorld.h>

#include <Scripting/ScriptDatabase.h>
#include <Scripting/ComponentDatabase.h>

#include "Ouroboros/ECS/GameObjectComponent.h"

namespace oo
{
    //forward declare
    class GameObject;
    

    class Scene : public IScene
    {
    public:
        using go_ptr = std::shared_ptr<oo::GameObject>;
        //using go_on_create_callback = std::function<void(go_ptr)>;
        
        // Events
    public:
        class OnInitEvent : public Event
        {
        };

    public:
        Scene(std::string_view name);
        virtual ~Scene();

        virtual void Init() override;
        virtual void Update() override;
        virtual void LateUpdate() override;
        virtual void Render() override;
        virtual void EndOfFrameUpdate() override;
        virtual void Exit() override;

        virtual void LoadScene() override;
        virtual void UnloadScene() override;
        virtual void ReloadScene() override;

        virtual LoadStatus GetProgress() const override;

    public:

        std::string GetFilePath() const;
        std::string GetSceneName() const;

        go_ptr CreateGameObjectDeferred(oo::UUID uuid = {});
        go_ptr CreateGameObjectImmediate(oo::UUID uuid = {});
        void DestroyGameObject(GameObject go);
        void DestroyGameObjectImmediate(GameObject go);
        
        go_ptr InstatiateGameObject(GameObject go);
        go_ptr DuplicateGameObject(GameObject go);

        // Attempts to search the lookup table with uuid.
        // returns the gameobject if it does
        // else returns nullptr.
        go_ptr FindWithInstanceID(oo::UUID uuid) const;
        bool IsValid(oo::UUID uuid) const;
        bool IsValid(GameObject go) const;

        /*
        void SwapChildren(GameObject const& other);
        */

        Ecs::ECSWorld& GetWorld();
        scenegraph const GetGraph() const;
        go_ptr GetRoot() const;
        GraphicsWorld* GetGraphicsWorld() const;
        go_ptr GetMainCameraObject() const;
        Camera MainCamera() const;

        // Graphics Specific code
        UUID GetUUIDFromPickingId(std::uint32_t pickingID) const;
        std::uint32_t GeneratePickingID(UUID uuid);
        void RemovePickingID(std::uint32_t pickingID);

    protected:
        void SetFilePath(std::string_view filepath);
        void SetSceneName(std::string_view name);

        void LoadFromFile();
        void SaveToFile();

        // Helper Functions
    private:
        go_ptr CreateGameObjectImmediate(go_ptr new_go);
        void InsertGameObject(go_ptr go_ptr);
        void RemoveGameObject(go_ptr go_ptr);

        // Old method [ keeping just to verify if something goes wrong one day ]
        //void RecusriveLinkScenegraph(GameObject original_parent_go, std::queue<Scene::go_ptr> new_objects);

        oo::UUID GetInstanceID(GameObject const& go) const;

        void OnEnableGameObject(GameObjectComponent::OnEnableEvent* e);
        void OnDisableGameObject(GameObjectComponent::OnDisableEvent* e);

        // Variables
    private:
        std::string m_name;
        std::string m_filepath;

        // set of gameobjects to initialize
        //std::vector<std::pair<go_ptr, go_on_create_callback>> m_createList;

        // set of ids to Remove 
        std::set<oo::UUID> m_removeList;
        // one copy of a lookup table for all gameobjects.
        std::map<oo::UUID, std::shared_ptr<oo::GameObject>> m_lookupTable;
        // direct copy of all gameobjects in the scene
        std::set<std::shared_ptr<oo::GameObject>> m_gameObjects;

        // graphics related ids
        std::unordered_map<std::uint32_t, UUID> m_pickingIdToUUID;
        std::unordered_map<UUID, std::uint32_t> m_uuidToPickingId;

        //TODO : temporarily only have one graphics world
        inline static std::unique_ptr<GraphicsWorld> m_graphicsWorld;
        std::unique_ptr<Ecs::ECSWorld> m_ecsWorld;
        std::unique_ptr<scenegraph> m_scenegraph;
        go_ptr m_rootGo;

        go_ptr m_mainCamera;

        // scripting stuff
        std::unique_ptr<ScriptDatabase> m_scriptDatabase;
        std::unique_ptr<ComponentDatabase> m_componentDatabase;
    };
}
