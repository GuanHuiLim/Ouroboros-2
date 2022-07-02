#include "pch.h"
#include "Scene.h"

namespace oo
{
    void Scene::Init()
    {
    }
    
    void Scene::Update()
    {
    }
    
    void Scene::LateUpdate()
    {
    }
    
    void Scene::Render()
    {
    }
    
    void Scene::EndOfFrameUpdate()
    {
    }
    
    void Scene::Exit()
    {
    }
    
    void Scene::LoadScene()
    {
    }
    
    void Scene::UnloadScene()
    {
    }
    
    void Scene::ReloadScene()
    {
    }

    LoadStatus Scene::GetProgress()
    {
        return LoadStatus();
    }
    
    void Scene::SetSaveFile(std::string_view filepath)
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
