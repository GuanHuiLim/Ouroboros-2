#pragma once

#include"TestScene.h"
#include <Archetypes_ECS/src/A_Ecs.h>

struct DummyComponent
{
    /*bool simple_test = true;
    std::size_t base_test = 999;
    std::size_t* fake_ptr = nullptr;*/

    std::string this_is_broken = "this is a test string long enough";
};

struct FakeGameObject;

struct FakeScene
{
    Ecs::ECSWorld world = {};
    std::map<std::size_t, std::shared_ptr<FakeGameObject>> m_cont;

    std::shared_ptr<FakeGameObject> CreateGO()
    {
        std::shared_ptr<FakeGameObject> fakego = std::make_shared<FakeGameObject>(this);
        std::size_t fake_uuid = 100;
        m_cont.emplace(fake_uuid, fakego);
        return fakego;
    }
};

struct FakeGameObject
{
    FakeScene* fakeScene = nullptr;
    Ecs::EntityID entity;
    FakeGameObject(FakeScene* scene)
        : fakeScene{ scene }
        , entity{ scene->world.new_entity<DummyComponent>() }
    {
    }
};

struct ECSTestScene final : public TestScene
{
    FakeScene m_fakeScene;
    
    ECSTestScene()
    {
        std::shared_ptr<FakeGameObject> fakeGo = m_fakeScene.CreateGO();
    }

};
