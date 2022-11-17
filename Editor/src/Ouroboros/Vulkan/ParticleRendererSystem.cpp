/************************************************************************************//*!
\file           ParticleRendererSystem.cpp
\project        Ouroboros
\author         
\par            
\date           Sept 30, 2022
\brief          Renderer System is in charge of putting all rendering related components
                and performing the correct instructions in order to render the expected
                scene properly

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "ParticleRendererSystem.h"
#include <rttr/registration>

#include <OO_Vulkan/src/DebugDraw.h>

#include "Ouroboros/Core/Application.h"
#include "VulkanContext.h"
#include "Ouroboros/Core/Timer.h"
#include "Ouroboros/Core/Application.h"
#include "Utility/Random.h"
#include "Ouroboros/Vulkan/ParticleProperties.h"
#include "Ouroboros/Vulkan/ParticleEmitterComponent.h"

#include "Ouroboros/EventSystem/EventTypes.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/Core/Input.h"

#include "App/Editor/UI/Object Editor/EditorViewport.h"
#include "Ouroboros/EventSystem/EventTypes.h"
#include "Ouroboros/ECS/GameObjectComponent.h"
#include "Ouroboros/Vulkan/GlobalRendererSettings.h"

namespace oo
{
    using ParticlePersistence = ParticleEmitterComponent::ParticlePersistence;
    void ParticleRendererSystem::SpawnParticles(ParticleEmitterComponent& emitter,TransformComponent& trans, uint32_t count)
    {
        const auto& partProps = emitter.m_partProperties;
        // go through and try to init
        for (size_t i = emitter.m_liveParticles; i < emitter.m_maxParticles; i++)
        {
            if (count == 0) break;

            if (emitter.m_persistentData[i].m_alive == false)
            {
                auto& shape = emitter.m_partShape;
                auto& pd = emitter.m_persistentData[i];
                float spd = emitter.m_partProperties.startSpeed;
                pd.m_speed = random::generate<float>(1.0f,std::move(spd));


                pd.m_startPos = (partProps.localSpace == true) ?  glm::vec3{} : trans.GetGlobalPosition();
                pd.m_scale = trans.GetGlobalScale();
                // TODO FIX QUATERION STUFF
                //pd.m_startRotation = trans.GetGlobalRotationRad();
                pd.m_startRotation = trans.GetGlobalRotationRad().x;


                switch (shape.shape)
                {
                case ParticleShape::Cone:
                {
                    float halfAngle = glm::radians(shape.angle)/ 2.0f;
                    float val = random::generate<float>(-std::move(halfAngle),std::move(halfAngle));
                    glm::vec2 dir = glm::vec2{val,cosf(val)};	
                    pd.m_rotationOffset = -val;
                    glm::mat2 rotMat = trans.GetGlobalRotationMatrix();
                    float halfDist = shape.size / 2.0f;
                    float dist = random::generate<float>(-std::move(halfDist), std::move(halfDist));
                    pd.m_startOffset = glm::vec3((rotMat * glm::vec2(dist, 0.0f)),0.0f);
                    pd.m_startDirection = glm::vec3{ dir, 0.0f };
                }
                break;
                case ParticleShape::Circle:
                {
                    float val = random::generate<float>(0.0f,2*glm::pi<float>());
                    pd.m_startDirection = glm::vec3{cosf(val),sinf(val),0.0f};	
                    pd.m_rotationOffset= val - glm::pi<float>()/2.0f;	
                    glm::mat3 rotMat = trans.GetGlobalRotationMatrix();
                    pd.m_startOffset = rotMat* pd.m_startDirection * shape.size;
                }
                break;
                default:
                {
                    float val = random::generate<float>(0.0f,2*glm::pi<float>());
                    pd.m_startDirection = glm::vec3{cosf(val),sinf(val),0.0f};	
                }
                break;
                }

                pd.m_maxLifetime = partProps.maxLifetime;
                pd.m_alive = true;

                --count;
                ++emitter.m_liveParticles;
            }
        }
    }
    void ParticleRendererSystem::UpdateAllParticlesLifetime(ParticleEmitterComponent& emitter, float deltaTime)
    {
        for(size_t i = 0; i < emitter.m_maxParticles; i++)
        {
            UpdateParticleLifetime(emitter,deltaTime,i);
        }
    }

    void ParticleRendererSystem::UpdateParticleLifetime(ParticleEmitterComponent& emitter, float deltaTime, size_t index)
    {
        if (index >= emitter.m_maxParticles) return;

        //perform update here
        auto& pp = emitter.m_persistentData[index];
        if (pp.m_alive == true)
        {
            pp.m_lifetime += deltaTime;
            if (pp.m_lifetime > pp.m_maxLifetime)
            {
                // particle dead.. reset particle
                pp = ParticlePersistence();

                --emitter.m_liveParticles;
            }
        }
    }

    void ParticleRendererSystem::SimulateSingleParticle(ParticleEmitterComponent& emitter, TransformComponent& trans, float deltaTime, size_t index)
    {
        if (index >= emitter.m_maxParticles) return;

        const auto& partProps = emitter.m_partProperties;
        auto& pD = emitter.m_persistentData[index];
        auto& particle = emitter.m_particles[index]; 
        auto t = pD.m_lifetime / pD.m_maxLifetime;

        InterpolatedValues interp{
            InterpolateVector(partProps.p_colours, partProps.colours, t),
            InterpolateVector(partProps.p_speeds, partProps.speeds, t),
            glm::radians(InterpolateVector(partProps.p_rotations, partProps.rotations, t)),
            InterpolateVector(partProps.p_directions, partProps.directions, t),
            InterpolateVector(partProps.p_sizes, partProps.sizes, t)
        };	

        interp.direction = pD.m_startDirection + interp.direction;
        float len = glm::length(interp.direction);
        if (len > 0.0f)
        {
            //make sure we are not normalizing zero vectors
            interp.direction /= len;
        }
        // accumlate the delta for the particle position
        pD.m_posDelta += interp.direction*  trans.GetGlobalScale() * deltaTime * interp.speed;

        glm::vec3 pos;		
        glm::vec3 pSize = trans.GetGlobalScale() * interp.size;
        glm::mat4 iden = glm::mat4(1.0f);

        //particle.colour = partProps.col_b.asV() * t + partProps.col_a.asV() * t_1;
        particle.colour = interp.col;
        particle.instanceData.x = 1u;
        // TODO get entity somehow
        //particle.entityID = trans.GetEntity();

        if (emitter.m_partProperties.localSpace == true)
        {
            // we need to multiply the position delta by the relevant rotation matrix to get the correct position in the world
            //auto rotMat = glm::rotation_z_matrix(trans.GetGlobalRotationRad() + interp.rotation + pD.m_rotationOffset);
            pos = trans.GetGlobalPosition() + pD.m_startOffset + (glm::mat3(trans.GetGlobalRotationMatrix())* pD.m_posDelta);

            glm::mat4 fxform = glm::mat4(1.0f);
            fxform= glm::translate(fxform,pos);
            fxform= glm::rotate(fxform, trans.GetGlobalRotationRad().x + pD.m_rotationOffset + interp.rotation, glm::vec3{0.0f,1.0f,0.0f});
            fxform= glm::scale(fxform,pSize);
            particle.transform = fxform;
            return;
        }

        //auto rotMat = glm::rotation_z_matrix(pD.m_startRotation + interp.rotation + pD.m_rotationOffset);
        pos = pD.m_startPos + pD.m_startOffset + (glm::mat3(trans.GetGlobalRotationMatrix())*pD.m_posDelta);

        glm::mat4 fxform = glm::mat4(1.0f);
        fxform= glm::translate(fxform,pos);
        //TODO: FIX QUATERNION
        fxform= glm::rotate(fxform, trans.GetGlobalRotationRad().x + pD.m_rotationOffset + interp.rotation, glm::vec3{0.0f,1.0f,0.0f});
        fxform= glm::scale(fxform,pSize);
        particle.transform = fxform;
    }

    void ParticleRendererSystem::PerformBulkPrewarm(ParticleEmitterComponent& emitter, TransformComponent& trans)
    {
        auto& partProps = emitter.m_partProperties;
        uint32_t particlesCnt = std::min(emitter.m_maxParticles, static_cast<uint32_t>(std::floorf(partProps.maxLifetime / emitter.m_spawnRate)));
        SpawnParticles(emitter, trans, particlesCnt);

        if (emitter.bakedData.size() < particlesCnt)
        {
            emitter.m_prewarmBaked = false;
            return;
        }

        for (size_t i = 0; i < particlesCnt; ++i)
        {
            emitter.m_persistentData[i].m_posDelta = emitter.bakedData[i].posDelta;
            emitter.m_persistentData[i].m_lifetime = emitter.bakedData[i].lifetime;
        }

        emitter.m_prewarmPerformed = true;
        //end prewarm
    }

    void ParticleRendererSystem::BakePrewarm(ParticleEmitterComponent& emitter, TransformComponent& trans)
    {
        // TODO: profiling
        //static thread_local const char* lc = "BakePrewarm";
        //TracyCZoneN(__FUNC__,lc, tracy::Color::Orange);

        emitter.ResetSystem();
        const auto& partProps = emitter.m_partProperties;
        //spawn the amount needed
        uint32_t particlesCnt = std::min(emitter.m_maxParticles, static_cast<uint32_t>(std::floorf(partProps.maxLifetime / emitter.m_spawnRate)));

        emitter.bakedData.resize(particlesCnt);
        std::fill(emitter.bakedData.begin(), emitter.bakedData.end(), ParticleEmitterComponent::BakedData{});

        SpawnParticles(emitter, trans, particlesCnt);
        //do a backwards iniitliazation of particles
        float bulkTime = partProps.maxLifetime - particlesCnt * emitter.m_spawnRate;
        //create a global timeslice
        constexpr float fixedDT = 0.02f;
        float time = fixedDT;

        while (time < bulkTime)
        {
            float t = time / partProps.maxLifetime;
            auto interpDirection = InterpolateVector(partProps.p_directions, partProps.directions, t);
            auto interpSpeed = InterpolateVector(partProps.p_speeds, partProps.speeds, t);
            for (size_t i = 0; i < particlesCnt; i++)
            {
                auto& pD = emitter.m_persistentData[i];
                auto& particle = emitter.m_particles[i];

                auto dir = pD.m_startDirection + interpDirection;
                float len = glm::length(dir);
                if (len > 0.0f)
                {
                    //make sure we are not normalizing zero vectors
                    dir /= len;
                }
                // accumlate the delta for the particle position
                pD.m_posDelta += dir * trans.GetGlobalScale() * fixedDT * interpSpeed;
                emitter.bakedData[i].posDelta = pD.m_posDelta;

            }
            time += fixedDT;
        }
        while (time < partProps.maxLifetime)
        {
            float t = time / partProps.maxLifetime;
            size_t ignorePart = static_cast<size_t>(particlesCnt * (time - bulkTime) / (partProps.maxLifetime - bulkTime));
            auto interpDirection = InterpolateVector(partProps.p_directions, partProps.directions, t);
            auto interpSpeed = InterpolateVector(partProps.p_speeds, partProps.speeds, t);

            for (size_t i = particlesCnt - 1; i > ignorePart; --i)
            {
                auto& pD = emitter.m_persistentData[i];
                auto& particle = emitter.m_particles[i];

                auto dir = pD.m_startDirection + interpDirection;
                float len = glm::length(dir);
                if (len > 0.0f)
                {
                    //make sure we are not normalizing zero vectors
                    dir /= len;
                }
                // accumlate the delta for the particle position
                pD.m_posDelta += dir * trans.GetGlobalScale() * fixedDT * interpSpeed;
                emitter.bakedData[i].posDelta = pD.m_posDelta;
                emitter.bakedData[i].lifetime = time;
            }
            time += fixedDT;
        }

        //spawn the amount needed
        emitter.m_prewarmBaked = true;

        //TracyCZoneEnd(__FUNC__);
    }


    void ParticleRendererSystem::SimulateAllParticles(ParticleEmitterComponent& emitter,TransformComponent& trans, float deltaTime)
    {	
        for (size_t i = 0; i < emitter.m_liveParticles; i++)
        {
            SimulateSingleParticle(emitter, trans, deltaTime,i);
        }
    }

    void ParticleRendererSystem::OnEmitterAssign(Ecs::ComponentEvent<ParticleEmitterComponent>* evnt)
    {
        assert(m_world != nullptr); // it should never be nullptr, was the Init funciton called?

        auto& emitterComp = evnt->component;
        auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);
        InitializeEmitter(emitterComp, transform_component);

        // TODO: HARDCODED DEFAULTS : CURRENTLY ASSIGNED CUBE, TO BE REMOVED LATER
        emitterComp.m_partRenderer.ModelHandle = 0;
        emitterComp.m_partRenderer.MeshInformation.submeshBits[0] = true;
    }

    void ParticleRendererSystem::OnEmitterRemove(Ecs::ComponentEvent<ParticleEmitterComponent>* evnt)
    {
        auto& comp = evnt->component;
        m_graphicsWorld->DestroyEmitterInstance(comp.GraphicsWorldID);
    }



    oo::ParticleRendererSystem::ParticleRendererSystem(GraphicsWorld* graphicsWorld, Scene* scene)
        : m_graphicsWorld { graphicsWorld }
        , m_scene { scene }
    {
        assert(graphicsWorld != nullptr);	// it should never be nullptr, who's calling this?
    }

    ParticleRendererSystem::~ParticleRendererSystem()
    {
    }

    void ParticleRendererSystem::Init()
    {
        // Emitter
        m_world->SubscribeOnAddComponent<ParticleRendererSystem, ParticleEmitterComponent>(
            this, &ParticleRendererSystem::OnEmitterAssign);

        m_world->SubscribeOnRemoveComponent<ParticleRendererSystem, ParticleEmitterComponent>(
            this, &ParticleRendererSystem::OnEmitterRemove);

    }

    void ParticleRendererSystem::SaveEditorCamera()
    {
        // Save camera information back to appropriate place
        oo::GetCurrentSceneStateEvent e;
        oo::EventManager::Broadcast(&e);
        switch (e.state)
        {
        case oo::SCENE_STATE::RUNNING:
            EditorViewport::EditorCamera = m_graphicsWorld->cameras[1];
            break;
        }
        //EditorController::EditorCamera  = Application::Get().GetWindow().GetVulkanContext()->getRenderer()->camera;
    }

    void oo::ParticleRendererSystem::Run(Ecs::ECSWorld* world)
    {
        // Update Newly Duplicated Emitter
        static Ecs::Query duplicated_emitter_query = Ecs::make_raw_query<ParticleEmitterComponent, TransformComponent, DuplicatedComponent>();
        world->for_each(duplicated_emitter_query, [&](ParticleEmitterComponent& emitterComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
        { 
            InitializeEmitter(emitterComp, transformComp);
        });

        // update Emitter
        static Ecs::Query emitter_query = Ecs::make_query<ParticleEmitterComponent, TransformComponent>();
        world->for_each(emitter_query, [&](ParticleEmitterComponent& emitter, TransformComponent& transformComp) 
        {
            emitter.m_partRenderer.MeshInformation.submeshBits[0] = true;
            //do nothing if transform did not change
            auto& actualObject = m_graphicsWorld->GetEmitterInstance(emitter.GraphicsWorldID);
            actualObject.modelID = emitter.m_partRenderer.ModelHandle;
            actualObject.bindlessGlobalTextureIndex_Albedo      = emitter.m_partRenderer.AlbedoID;
            actualObject.bindlessGlobalTextureIndex_Normal      = emitter.m_partRenderer.NormalID;
            actualObject.bindlessGlobalTextureIndex_Metallic    = emitter.m_partRenderer.MetallicID;
            actualObject.bindlessGlobalTextureIndex_Roughness   = emitter.m_partRenderer.RoughnessID;
            actualObject.submesh = emitter.m_partRenderer.MeshInformation.submeshBits;

            if (transformComp.HasChangedThisFrame)
                actualObject.localToWorld = transformComp.GlobalTransform;
            
            // update transform if this is the first frame of rendering
            if (m_firstFrame)
            {
                actualObject.localToWorld = transformComp.GlobalTransform;
                m_firstFrame = false;
            }

            if (emitter.m_prewarm == true && emitter.m_prewarmPerformed == false)
            {
                // TODO: we need a bake button
                if (emitter.m_prewarmBaked == false || emitter.bakedData.empty())
                {
                    // perform the bake
                    BakePrewarm(emitter, transformComp);
                    emitter.ResetSystem();
                }
                PerformBulkPrewarm(emitter, transformComp);
            }

            emitter.m_systemLifetime += FixedDeltaTime;
            emitter.m_spawnCooldown += FixedDeltaTime;

            if (emitter.m_looping == false && emitter.m_systemLifetime > emitter.m_duration)
            {
                emitter.m_playing = false;
            }

            //spawn as many particles as needed
            int toSpawnCnt = 0;
            while (emitter.m_spawnCooldown > emitter.m_spawnRate)
            {
                // TODO: Burst mode
                emitter.m_spawnCooldown -= emitter.m_spawnRate;
                ++toSpawnCnt;
            }

            if (emitter.m_playing == false)
            {
                toSpawnCnt = 0;
            }

            // update all so we can have dead particles
            UpdateAllParticlesLifetime(emitter, FixedDeltaTime);

            std::sort(emitter.m_persistentData.begin(), emitter.m_persistentData.end(), [](ParticlePersistence& l, ParticlePersistence& r)
                {
                    return l.m_alive > r.m_alive;
                }
            );

            SpawnParticles(emitter, transformComp, toSpawnCnt);

            SimulateAllParticles(emitter, transformComp, FixedDeltaTime);
        });
           
        world->for_each(emitter_query, [&](ParticleEmitterComponent& emitter, TransformComponent& transformComp)
        {
            m_graphicsWorld->SubmitParticles(emitter.m_particles, emitter.m_liveParticles, emitter.GraphicsWorldID);
        });
    }

    void ParticleRendererSystem::InitializeEmitter(ParticleEmitterComponent& emitterComp, TransformComponent& transformComp)
    {
        emitterComp.GraphicsWorldID = m_graphicsWorld->CreateEmitterInstance();
        
        // update graphics world side
        auto& graphics_object = m_graphicsWorld->GetEmitterInstance(emitterComp.GraphicsWorldID);
        graphics_object.localToWorld = transformComp.GetGlobalMatrix();
        graphics_object.entityID = 1;
    }

}

