/************************************************************************************//*!
\file           ECSTestScene.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes a test scene that uses ecs stuff for scene-testing purposes

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include"TestScene.h"
#include <Archetypes_ECS/src/A_Ecs.h>
#include "Ouroboros/Core/Base.h"
#include "Ouroboros/ECS/ECS.h"

struct DummyComponent
{
    std::string dynamic_allocated_data_types_ok = "this is a test string long enough";
};

struct DummySystem : public Ecs::System
{
    void Update(DummyComponent& comp)
    {
        // do nothing.
        UNREFERENCED(comp);

        // work on the data
        comp.dynamic_allocated_data_types_ok = "change it to something else";
    }

public:

    virtual void Run(Ecs::ECSWorld * world) override
    {
        static Ecs::Query query = Ecs::make_raw_query<DummyComponent>();
        world->for_each(query, [&](DummyComponent& comp) { Update(comp); });
    }
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

    void Init()
    {
        // Problematic Statement #1
        //world.Add_System<DummySystem>();
    }

    void Update()
    {
        // Problematic Statement #2
        //world.Get_System<DummySystem>()->Run(&world);
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
        
        // inspect these functions for problematic statements
        m_fakeScene.Init();
        m_fakeScene.Update();
    }

};
