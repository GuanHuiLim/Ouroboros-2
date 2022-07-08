#include "pch.h"
#include "Scene.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/Core/Log.h"

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
    #define PRINT(name) std::cout << "[" << (name) << "] : " << __FUNCTION__ << std::endl;
#else
    #define PRINT(name) 
#endif

namespace oo
{
    void Scene::Init()
    {
        Scene::OnInitEvent e;
        EventManager::Broadcast(&e);

        PRINT(m_name);
    }
    
    void Scene::Update()
    {
        PRINT(m_name);
    }
    
    void Scene::LateUpdate()
    {
        PRINT(m_name);
    }
    
    void Scene::Render()
    {
        PRINT(m_name);
    }
    
    void Scene::EndOfFrameUpdate()
    {
        PRINT(m_name);

        // go through all things to remove at the end of frame and do so.
        for (auto& uuid : m_removeList)
        {
            auto go_ptr = FindWithInstanceID(uuid);
            ASSERT_MSG
            (
                (go_ptr == nullptr),
                "Attempting to delete an object that's already been removed"
            );

            // Actual Deletion
            m_ecsWorld.destroy(go_ptr->GetEntity());
            m_lookupTable.erase(uuid);
        }
        m_removeList.clear();
    }
    
    void Scene::Exit()
    {
        PRINT(m_name);
    }
    
    void Scene::LoadScene()
    {
        PRINT(m_name);
    }
    
    void Scene::UnloadScene()
    {
        PRINT(m_name);
    }
    
    void Scene::ReloadScene()
    {
        PRINT(m_name);
    }

    LoadStatus Scene::GetProgress()
    {
        PRINT(m_name);
        return LoadStatus();
    }
    
    void Scene::SetFilePath(std::string_view filepath)
    {
        m_filepath = filepath;
    }
    
    std::string Scene::GetFilePath() const 
    { 
        return m_filepath; 
    }
    
    void Scene::SetSceneName(std::string_view name) 
    { 
        m_name = name; 
    }
    
    std::string Scene::GetSceneName() const 
    { 
        return m_name; 
    }

    oo::GameObject Scene::CreateGameObject()
    {
        oo::GameObject newObject{ &m_ecsWorld };
        // IMPT: we are using the uuid to retrieve back the gameobject as well!
        newObject.GetComponent<GameObjectComponent>().Node = m_scenegraph.create_new_child(newObject.Name(), newObject.GetInstanceID());
        m_lookupTable.emplace(newObject.GetInstanceID(), std::make_shared<GameObject>(newObject));

        return newObject;
    }

    std::shared_ptr<GameObject> Scene::FindWithInstanceID(UUID uuid)
    {
        if (m_lookupTable.contains(uuid))
            return m_lookupTable.at(uuid);

        return nullptr;
    }

    bool Scene::IsValid(GameObject go) const
    {
        // a valid gameobject will not have its ID as NotFound, will have gameobject component(minimally)
        // and parent will not be equals to NOParent

        return !(go.GetEntity().value == GameObject::NOTFOUND.value
            || go.HasComponent<GameObjectComponent>() == false
            || go.GetWorld() == nullptr
            || go.GetWorld() != &m_ecsWorld);
    }

    void Scene::DestroyGameObject(GameObject go)
    {
        ASSERT_MSG(IsValid(go), "Working on an invalid GameObject");

        // safety check
        if (FindWithInstanceID(go.GetInstanceID()) == nullptr)
        {
            LOG_CORE_ERROR("Attempting to remove an invalid gameobject {0} with instance ID of {1}", go.GetEntity().value, go.GetInstanceID());
            return;
        }

        // Queue selected node for deletion
        m_removeList.emplace(go.GetInstanceID());

        // Then continue to destroy all its children
        for (auto const& childuuid : go.GetChildrenUUID())
        {
            auto go = FindWithInstanceID(childuuid);
            if(go)
                DestroyGameObject(*go);
            else            
                LOG_CORE_ERROR("attempting to remove an invalid uuid {0} from scene {1}", childuuid, m_name);
        }
    }
    
    void Scene::LoadFromFile()
    {
    }

    void Scene::SaveToFile()
    {
    }
    
    Ecs::ECSWorld Scene::GetWorld() const
    {
        return m_ecsWorld;
    }

    scenegraph Scene::GetGraph() const
    {
        return m_scenegraph;
    }

    oo::GameObject Scene::GetRoot() const
    {
        return m_root;
    }
}
