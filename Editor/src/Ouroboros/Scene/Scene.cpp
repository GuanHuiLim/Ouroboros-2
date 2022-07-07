#include "pch.h"
#include "Scene.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/Core/Log.h"
#define PRINT(name) std::cout << "[" << (name) << "] : " << __FUNCTION__ << std::endl;

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
    
    void Scene::LoadFromFile()
    {
    }

    void Scene::SaveToFile()
    {
    }
}
