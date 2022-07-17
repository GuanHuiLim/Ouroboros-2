#pragma once

#include <SceneManagement/include/IScene.h>
#include "Ouroboros/EventSystem/Event.h"

#include <scenegraph/include/scenegraph.h>
#include <Archetypes_ECS/src/A_Ecs.h>
#include <set>
#include "Utility/UUID.h"

namespace oo
{
    //forward declare
    class GameObject;

    class Scene : public IScene
    {
        // Events
    public:
        class OnInitEvent : public Event
        {
        };

    public:
        Scene(std::string_view name);
        virtual ~Scene() = default;

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

        std::shared_ptr<GameObject> CreateGameObject();

        // Attempts to search the lookup table with uuid.
        // returns the gameobject if it does
        // else returns nullptr.
        std::shared_ptr<GameObject> FindWithInstanceID(UUID uuid);
        
        bool IsValid(GameObject go) const;
        void DestroyGameObject(GameObject go);
        void DestroyGameObjectImmediate(GameObject go);

        std::shared_ptr<GameObject> InstatiateGameObject(GameObject go);
        /*void AddChild(GameObject const& gameObj, bool preserveTransforms = false) const;
        void AddChild(std::initializer_list<GameObject> gameObjs, bool preserveTransforms = false) const;
        void SwapChildren(GameObject const& other);
        GameObject GetParent() const;
        std::vector<Entity> GetDirectChilds(bool includeItself = false) const;
        std::vector<Entity> GetChildren(bool includeItself = false) const;*/

        Ecs::ECSWorld& GetWorld();

        scenegraph GetGraph() const;
        std::shared_ptr<GameObject> GetRoot() const;

    protected:
        void SetFilePath(std::string_view filepath);
        void SetSceneName(std::string_view name);

        void LoadFromFile();
        void SaveToFile();

    private:
        std::shared_ptr<GameObject> CreateGameObject(std::shared_ptr<GameObject> new_go);
        void InsertGameObject(std::shared_ptr<GameObject> go_ptr);
        void RemoveGameObject(std::shared_ptr<GameObject> go_ptr);

        std::string m_name;
        std::string m_filepath;

        // set of ids to Remove 
        std::set<UUID> m_removeList;
        // one copy of a lookup table for all gameobjects.
        std::map<UUID, std::shared_ptr<oo::GameObject>> m_lookupTable;
        // direct copy of all gameobjects in the scene
        std::set<std::shared_ptr<oo::GameObject>> m_gameObjects;

        Ecs::ECSWorld m_ecsWorld;
        std::unique_ptr<scenegraph> m_scenegraph;
        std::shared_ptr<GameObject> m_rootGo;
    };
}
