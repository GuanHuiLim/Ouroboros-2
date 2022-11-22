/************************************************************************************//*!
\file           ParticleEmitter.h
\project        Ouroboros
\author         
\par            
\date           Sept 30, 2022
\brief          Describes the Mesh Renderer and Skin Mesh Renderer Components that'll
                be the interface for users to fill up and used by renderer system.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "glm/common.hpp"
#include "Ouroboros/Asset/AssetManager.h"
#include "MeshInfo.h"
#include "ParticleProperties.h"
#include <rttr/type>
#include "OO_Vulkan/src/GraphicsWorld.h"

namespace oo
{
    class Material;
    
    constexpr uint32_t ARB_MAX_PARTICLE_COUNT_PER_SYSTEM = 100000u;
    
    struct ParticleEmitterComponent
    {
        ParticleEmitterComponent();
        ~ParticleEmitterComponent();

        struct ParticlePersistence
        {
            bool m_alive = false;

            float m_lifetime = 0.0f;
            float m_maxLifetime = 5.0f;

            glm::vec3 m_startDirection{0.0f};
            glm::vec3 m_scale{1.0f};
            glm::vec3 m_posDelta{};
            glm::vec3 m_startPos{};
            glm::vec3 m_startOffset{};

            float m_speed{};

            float m_startRotation{};
            float m_rotationOffset{};
        };

        uint32_t m_maxParticles;

        // SYSTEM PROPERTIES
        float m_spawnRate;
        float m_systemLifetime;
        float m_duration;
        float m_spawnCooldown;

        bool m_looping;
        bool m_prewarm;

        bool m_prewarmPerformed;

        uint32_t m_liveParticles;

        ParticleProperties m_partProperties;

        ParticlePropsShape m_partShape;
        ParticlePropsRenderer m_partRenderer;

        std::vector<ParticlePersistence> m_persistentData;
        std::vector<ParticleData> m_particles;

        // TODO: this is expensive in terms of memory, will either move out into a baked file
        //		 or else will try to optimize in some other way.
        bool m_prewarmBaked = false;

        bool m_playing = true;
        // SYSTEM PROPERTIES

        struct BakedData{
            glm::vec3 posDelta{};
            float lifetime{};
        };
        std::vector<BakedData> bakedData;

        //no need to serialize
        uint32_t GraphicsWorldID{};


        /// <summary>
        /// Functions for emitter
        /// </summary>
        uint32_t GetMaxParticles()const;
        void SetMaxParticles(uint32_t p);

        float GetParticleRate()const;
        void SetParticleRate(float r);

        float GetDuration()const;
        void SetDuration(float f);

        float GetMaxLifetime()const;
        void SetMaxLifetime(float f);

        const ParticleProperties& GetParticleProperties()const;
        void SetParticleProperties(const ParticleProperties& pp);

        const ParticlePropsRenderer& GetParticleRendererProperties()const;
        void SetParticleRendererProperties(const ParticlePropsRenderer& pp);

        const ParticlePropsShape& GetParticleShapeProperties()const;
        void SetParticleShapeProperties(const ParticlePropsShape& pp);

        bool GetLooping()const;
        void SetLooping(bool s);

        bool GetPrewarm()const;
        void SetPrewarm(bool s);

        bool GetPrewarmBaked()const;
        void SetPrewarmBaked(bool s);

        void Play();
        void Stop();

        bool GetPlaying();
        void SetPlaying(bool s);

        void ResetSystem();

        void BakePrewarm(bool s);
        std::vector<BakedData> GetPrewarmBakedData();
        void SetPrewarmBakedData(std::vector<BakedData> data);

        RTTR_ENABLE();
    };

}