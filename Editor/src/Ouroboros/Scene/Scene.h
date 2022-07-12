#pragma once

#include <IScene.h>
#include "Ouroboros/EventSystem/Event.h"

#include <scenegraph.h>
#include <A_Ecs.h>
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

        virtual LoadStatus GetProgress() override;

        std::string GetFilePath() const;
        std::string GetSceneName() const;

        std::weak_ptr<GameObject> CreateGameObject();

        // Attempts to search the lookup table with uuid.
        // returns the gameobject if it does
        // else returns max.
        std::shared_ptr<GameObject> FindWithInstanceID(UUID uuid);
        
        bool IsValid(GameObject go) const;
        void DestroyGameObject(GameObject go);

        /*void AddChild(GameObject const& gameObj, bool preserveTransforms = false) const;
        void AddChild(std::initializer_list<GameObject> gameObjs, bool preserveTransforms = false) const;
        void SwapChildren(GameObject const& other);
        GameObject GetParent() const;
        std::vector<Entity> GetDirectChilds(bool includeItself = false) const;
        std::vector<Entity> GetChildren(bool includeItself = false) const;*/

        Ecs::ECSWorld& GetWorld();
        scenegraph GetGraph() const;
        GameObject* GetRoot() const;

    protected:
        void SetFilePath(std::string_view filepath);
        void SetSceneName(std::string_view name);

        void LoadFromFile();
        void SaveToFile();

    private:
        std::string m_name;
        std::string m_filepath;

        // set of ids to Remove 
        std::set<UUID> m_removeList;
        // one copy of a lookup table for all gameobjects.
        std::map<UUID, std::shared_ptr<oo::GameObject>> m_lookupTable;

        Ecs::ECSWorld m_ecsWorld;
        scenegraph m_scenegraph;
        std::shared_ptr<GameObject> m_root;
    };
}
