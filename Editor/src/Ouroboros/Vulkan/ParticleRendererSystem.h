/************************************************************************************//*!
\file           ParticleRendererSystem.h
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
#pragma once

#include "ParticleEmitterComponent.h"
#include "CameraComponent.h"

#include "Archetypes_Ecs/src/A_Ecs.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"

#include "Ouroboros/Core/Timer.h"

#include "Ouroboros/Core/Events/ApplicationEvent.h"
#include "Ouroboros/Scene/Scene.h"

namespace oo
{

    class ParticleRendererSystem : public Ecs::System
    {
    public:
        ParticleRendererSystem(GraphicsWorld* graphicsWorld, Scene* scene);
        virtual ~ParticleRendererSystem();

        void Init();

        virtual void Run(Ecs::ECSWorld* world) override;

        void SaveEditorCamera();

        inline static bool CameraDebugDraw = true;
    private:
        template<typename T>
        T InterpolateVector(const std::vector<float>& perc, const std::vector<T>& items, float progress);

        void OnEmitterAssign(Ecs::ComponentEvent<ParticleEmitterComponent>* evnt);
        void OnEmitterRemove(Ecs::ComponentEvent<ParticleEmitterComponent>* evnt);

        void InitializeEmitter(ParticleEmitterComponent& emitterComp, TransformComponent& transformComp);

        void SpawnParticles(ParticleEmitterComponent& emitter,TransformComponent& trans,uint32_t count);
        void UpdateAllParticlesLifetime(ParticleEmitterComponent& emitter,float deltaTime);
        void UpdateParticleLifetime(ParticleEmitterComponent& emitter,float deltaTime, size_t index);
        void SimulateAllParticles(ParticleEmitterComponent& emitter,TransformComponent& trans, float deltaTime);
        void SimulateSingleParticle(ParticleEmitterComponent& emitter,TransformComponent& trans, float deltaTime, size_t index);

        void PerformBulkPrewarm(ParticleEmitterComponent& emitter, TransformComponent& trans);
        void BakePrewarm(ParticleEmitterComponent& emitter, TransformComponent& trans);

    private:
        using Timestep = double;
        inline static std::uint64_t MaxIterations = 100;
        inline static Timestep FixedDeltaTime = 1.0/MaxIterations;                 // physics updates at 100 fps
        inline static Timestep AccumulatorLimit = FixedDeltaTime * MaxIterations;  // To prevent spiral of death

        GraphicsWorld* m_graphicsWorld{ nullptr };
        uint32_t default_sprite_id{ static_cast<uint32_t>(-1) };
        Scene* m_scene;

        bool m_firstFrame = true; // potentially improvable if this can be run once per creation
    };


    template<typename T>
    inline T ParticleRendererSystem::InterpolateVector(const std::vector<float>& perc,const std::vector<T>& items, float progress)
    {
        // find the minimum size vector
        size_t smallest = std::min(perc.size(), items.size());
        if (smallest <= 1) return items.empty()? T(): items.front();

        // start from the back and find the smallest index
        size_t index = perc.size() - 1;
        while (index && perc[index] > progress) --index;

        if (index < smallest-1)
        {
            // interpolate between two values
            float liveProgress = (perc[index + 1]-progress ) / (perc[index + 1] - perc[index]);
            float t_1 = (1.0f - liveProgress);
            return items[index] * liveProgress + items[index + 1] * t_1;
        }
        else
        {
            return items[smallest-1];
        }
    }

}
