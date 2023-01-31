/************************************************************************************//*!
\file           ParticleEmitter.cpp
\project        Ouroboros
\author        
\par            email:
\date           Sept 30, 2022
\brief          Describes the Mesh Renderer and Skin Mesh Renderer Components that'll
                be the interface for users to fill up and used by renderer system.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "ParticleEmitterComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>
#include <glm/common.hpp>
#include <algorithm>

#include "ParticleProperties.h"

#include "OO_Vulkan/src/MeshModel.h"
#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
#include "OO_Vulkan/src/DefaultMeshCreator.h"

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

    registration::class_<ParticleProperties>("Particle properties")
        .property("Local simulation", &ParticleProperties::GetLocalSpace, &ParticleProperties::SetLocalSpace)
        .property("Colour percents", &ParticleProperties::p_colours) (metadata(UI_metadata::SAME_LINE_WITH_NEXT, 2))
        .property("Colours", &ParticleProperties::colours)
        //(metadata(UI_metadata::SAMELINE, 0))
        //.property("Speed", &ParticleProperties::GetSpeed, &ParticleProperties::SetSpeed)
        .property("Speed percents", &ParticleProperties::p_speeds)(metadata(UI_metadata::SAME_LINE_WITH_NEXT, 2))
        //(metadata(UI_metadata::SAMELINE, 1))
        .property("Speeds", &ParticleProperties::speeds)
        //(metadata(UI_metadata::SAMELINE, 0))
        .property("Rotation percents", &ParticleProperties::p_rotations)(metadata(UI_metadata::SAME_LINE_WITH_NEXT, 2))
        //(metadata(UI_metadata::SAMELINE, 1))
        .property("Rotations", &ParticleProperties::rotations)
        //(metadata(UI_metadata::SAMELINE, 0))
        .property("Direction percents", &ParticleProperties::p_directions)(metadata(UI_metadata::SAME_LINE_WITH_NEXT, 2))
        //(metadata(UI_metadata::SAMELINE, 1))
        .property("Directions", &ParticleProperties::directions)
        //(metadata(UI_metadata::SAMELINE, 0))
        .property("Size percents", &ParticleProperties::p_sizes)(metadata(UI_metadata::SAME_LINE_WITH_NEXT, 2))
        //(metadata(UI_metadata::SAMELINE, 1))
        .property("Sizes", &ParticleProperties::sizes)
        //(metadata(UI_metadata::SAMELINE, 0))
        ;

    registration::class_<ParticlePropsRenderer>("Particle renderer")
        .property_readonly("Model Handle", &ParticlePropsRenderer::ModelHandle)
        .property("Albedo", &ParticlePropsRenderer::GetAlbedoMap, &ParticlePropsRenderer::SetAlbedoMap)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
        )
        .property("Normal", &ParticlePropsRenderer::GetNormalMap, &ParticlePropsRenderer::SetNormalMap)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
        )
        .property("Metallic", &ParticlePropsRenderer::GetMetallicMap, &ParticlePropsRenderer::SetMetallicMap)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
        )
        .property("Roughness", &ParticlePropsRenderer::GetRoughnessMap, &ParticlePropsRenderer::SetRoughnessMap)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture))
        )
        .property("Mesh", &ParticlePropsRenderer::GetMesh, &ParticlePropsRenderer::SetMesh)
        (
            metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Model))
        )
        .property("MeshInfo", &ParticlePropsRenderer::GetMeshInfo, &ParticlePropsRenderer::SetMeshInfo)
        // TODO: EMISSION
		//.property("Emission", &ParticlePropsRenderer::GetEmission, &ParticlePropsRenderer::SetEmission)
        ;
    registration::enumeration<ParticleShape>("ParticleShape")(
        value("Cone",ParticleShape::Cone),
        value("Circle", ParticleShape::Circle)
        );

	registration::enumeration<ParticlePropsRenderer::ParticleType>("ParticleType")(
		value("Billboard",ParticlePropsRenderer::ParticleType::BILLBOARD),
		value("Mesh", ParticlePropsRenderer::ParticleType::MESH)
		);

    registration::class_<ParticlePropsShape>("Particle shape")
        .property("Shape", &ParticlePropsShape::GetShape, &ParticlePropsShape::SetShape)
        .property("Size", &ParticlePropsShape::GetSize, &ParticlePropsShape::SetSize)
        .property("Angle", &ParticlePropsShape::GetAngle, &ParticlePropsShape::SetAngle)
        ;

		registration::class_<ParticleEmitterComponent>("Particle Emitter")
		.property_readonly("Graphics World ID", &ParticleEmitterComponent::GraphicsWorldID)
		.property("Randomize Position", &ParticleEmitterComponent::m_randomizePosition)
		.property("Randomize Direction", &ParticleEmitterComponent::m_randomizeStartDir)
		.property("Particle properties", &ParticleEmitterComponent::GetParticleProperties, &ParticleEmitterComponent::SetParticleProperties)
		.property("Particle renderer", &ParticleEmitterComponent::GetParticleRendererProperties, &ParticleEmitterComponent::SetParticleRendererProperties)
		.property("Particle shape", &ParticleEmitterComponent::GetParticleShapeProperties, &ParticleEmitterComponent::SetParticleShapeProperties)
		.property("Max particles", &ParticleEmitterComponent::GetMaxParticles, &ParticleEmitterComponent::SetMaxParticles)
		.property("Particle rate", &ParticleEmitterComponent::GetParticleRate, &ParticleEmitterComponent::SetParticleRate)
		(
			metadata(UI_metadata::DRAG_SPEED, 0.5f)
		)
		.property("Max lifetime", &ParticleEmitterComponent::GetMaxLifetime, &ParticleEmitterComponent::SetMaxLifetime)
		(
			metadata(UI_metadata::DRAG_SPEED, 0.1f)
		)
		.property("Duration", &ParticleEmitterComponent::GetDuration, &ParticleEmitterComponent::SetDuration)
		(
			metadata(UI_metadata::DRAG_SPEED, 0.1f)
		)
		//.property("Particle rate", &ParticleSystem::GetParticleRate, &ParticleSystem::SetParticleRate)
		.property("Looping", &ParticleEmitterComponent::GetLooping, &ParticleEmitterComponent::SetLooping)
		.property("Prewarm", &ParticleEmitterComponent::GetPrewarm, &ParticleEmitterComponent::SetPrewarm)
		.property("Prewarm Baked", &ParticleEmitterComponent::GetPrewarmBaked, &ParticleEmitterComponent::SetPrewarmBaked)
		.property("Bake", &ParticleEmitterComponent::GetPrewarmBaked, &ParticleEmitterComponent::BakePrewarm)
		.property("RO_BakedData", &ParticleEmitterComponent::GetPrewarmBakedData, &ParticleEmitterComponent::SetPrewarmBakedData)
		(
			metadata(UI_metadata::HIDDEN, true)
		)
		; 

    }

	ParticlePropsRenderer::ParticleType oo::ParticlePropsRenderer::GetParticleType()
	{
		return m_renderType;
	}

	void oo::ParticlePropsRenderer::SetParticleType(ParticleType ty)
	{ 
		m_renderType = ty; 
	}

	MeshInfo ParticlePropsRenderer::GetMeshInfo()
    {
        return MeshInformation;
    }

    /*********************************************************************************//*!
    \brief      this function will only set the submeshbits
    *//**********************************************************************************/
    void ParticlePropsRenderer::SetMeshInfo(MeshInfo info)
    {
        MeshInformation.submeshBits = info.submeshBits;
    }

    void ParticlePropsRenderer::SetModelHandle(Asset _asset, uint32_t _submodel_id)
    {
        MeshInformation.submeshBits.reset();
        MeshInformation.submeshBits[_submodel_id] = true;
        MeshInformation.mesh_handle = _asset;

        ModelHandle = MeshInformation.mesh_handle.GetData<ModelFileResource*>()->meshResource;
    }

    Asset ParticlePropsRenderer::GetMesh()
    {
        return MeshInformation.mesh_handle;
    }

    void ParticlePropsRenderer::SetMesh(Asset _asset)
    {
        if (_asset.IsValid())
        {
            MeshInformation.mesh_handle = _asset;
            ModelHandle = MeshInformation.mesh_handle.GetData<ModelFileResource*>()->meshResource;
            // HACK this is needed to render stuff under edit..
            // MeshInformation.submeshBits.reset();
            // MeshInformation.submeshBits[0] = true;
        }
    }

    void oo::ParticlePropsRenderer::SetAlbedoMap(Asset albedoMap)
    {
        AlbedoHandle = albedoMap;
        if (AlbedoHandle.IsValid())
        {
            AlbedoID = AlbedoHandle.GetData<uint32_t>();
        }
    }

    Asset oo::ParticlePropsRenderer::GetAlbedoMap() const
    {
        return AlbedoHandle;
    }

    void oo::ParticlePropsRenderer::SetNormalMap(Asset normalMap)
    {
        NormalHandle = normalMap;
        if (NormalHandle.IsValid())
        {
            NormalID = NormalHandle.GetData<uint32_t>();
        }
    }

    Asset oo::ParticlePropsRenderer::GetNormalMap() const
    {
        return NormalHandle;
    }

    void oo::ParticlePropsRenderer::SetMetallicMap(Asset metallicMap)
    {
        MetallicHandle = metallicMap;
        if (MetallicHandle.IsValid())
        {
            MetallicID = MetallicHandle.GetData<uint32_t>();
        }
    }

    Asset oo::ParticlePropsRenderer::GetMetallicMap() const
    {
        return MetallicHandle;
    }

    void oo::ParticlePropsRenderer::SetRoughnessMap(Asset roughnessMap)
    {
        RoughnessHandle = roughnessMap;
        if (RoughnessHandle.IsValid())
        {
            RoughnessID = RoughnessHandle.GetData<uint32_t>();
        }
    }

    Asset oo::ParticlePropsRenderer::GetRoughnessMap() const
    {
        return RoughnessHandle;
    }


	const ParticlePropsRenderer& oo::ParticleEmitterComponent::GetParticleRendererProperties() const
	{
		return m_partRenderer;
	}

	void oo::ParticleEmitterComponent::SetParticleRendererProperties(const ParticlePropsRenderer& pp)
	{
		m_partRenderer= pp;
	}

	const ParticlePropsShape& oo::ParticleEmitterComponent::GetParticleShapeProperties() const
	{
		return m_partShape;
	}

	void oo::ParticleEmitterComponent::SetParticleShapeProperties(const ParticlePropsShape& pp)
	{
		m_partShape = pp;
	}

	bool oo::ParticleEmitterComponent::GetLooping() const
	{
		return m_looping;
	}

	void oo::ParticleEmitterComponent::SetLooping(bool s)
	{
		m_looping = s;
	}

	bool oo::ParticleEmitterComponent::GetPrewarm() const
	{
		return m_prewarm;
	}

	void oo::ParticleEmitterComponent::SetPrewarm(bool s)
	{
		m_prewarm = s;
		if (m_prewarm == true)
		{
			ResetSystem();
		}
	}

	bool oo::ParticleEmitterComponent::GetPrewarmBaked() const
	{
		return m_prewarmBaked;
	}

	void oo::ParticleEmitterComponent::SetPrewarmBaked(bool s)
	{
		m_prewarmBaked = s;
		//m_prewarmPerformed = false;
	}

	void oo::ParticleEmitterComponent::Play()
	{
		m_systemLifetime = 0.0f;
		m_playing = true;
	}

	void oo::ParticleEmitterComponent::Stop()
	{
		m_playing = false;
	}

	bool oo::ParticleEmitterComponent::GetPlaying()
	{
		return m_playing;
	}

	void oo::ParticleEmitterComponent::SetPlaying(bool s)
	{
		m_playing = s;
	}

	void oo::ParticleEmitterComponent::BakePrewarm(bool s)
	{
		if (!s)
		{
			m_prewarmBaked = false;
			m_prewarmPerformed = false;
		}
	}

	std::vector<ParticleEmitterComponent::BakedData> oo::ParticleEmitterComponent::GetPrewarmBakedData() 
	{
		return bakedData;
	}

	void oo::ParticleEmitterComponent::SetPrewarmBakedData(std::vector<ParticleEmitterComponent::BakedData> data)
	{
		bakedData = data;
	}

	void oo::ParticleEmitterComponent::ResetSystem()
	{
		m_systemLifetime = 0.0f;
		m_spawnCooldown = 0.0f;
		m_liveParticles = 0u;
		m_prewarmPerformed = false;
		m_playing = true;
		//m_prewarmBaked = false;
		std::fill(m_persistentData.begin(), m_persistentData.end(), ParticlePersistence{});
	}

	oo::ParticleEmitterComponent::ParticleEmitterComponent():
		m_maxParticles{ 1000u },
		m_particles{ m_maxParticles },
		m_persistentData{ m_maxParticles },
		m_spawnRate{1.0f / 10.0f},
		m_duration{5.0f},
		m_systemLifetime{0.0f},
		m_liveParticles{0u},
		m_spawnCooldown{0.0f},
		m_looping{true},
		m_playing{true},
		m_prewarm{false},
		m_prewarmPerformed{false},
		m_prewarmBaked{false}
	{

	}

	oo::ParticleEmitterComponent::~ParticleEmitterComponent()
	{
		ResetSystem();
	}

	uint32_t ParticleEmitterComponent::GetMaxParticles()const 
	{
		return m_maxParticles;
	}
	void ParticleEmitterComponent::SetMaxParticles(uint32_t p)
	{
		m_maxParticles = std::min(p, ARB_MAX_PARTICLE_COUNT_PER_SYSTEM);
		m_liveParticles = std::min(m_liveParticles, m_maxParticles);
		m_particles.resize(m_maxParticles);
		m_persistentData.resize(m_maxParticles);
	}

	float ParticleEmitterComponent::GetParticleRate()const 
	{
		if (m_spawnRate == FLT_MAX)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f / m_spawnRate;
		}
	}

	void ParticleEmitterComponent::SetParticleRate(float r)
	{
		if (r == 0)
		{
			m_spawnRate = FLT_MAX;
		}
		else
		{
			m_spawnRate = 1.0f / std::max(r,std::numeric_limits<float>::epsilon());
		}
	}

	float oo::ParticleEmitterComponent::GetDuration() const
	{
		return m_duration;
	}

	void oo::ParticleEmitterComponent::SetDuration(float f)
	{
		m_duration = f;
	}

	const ParticleProperties& ParticleEmitterComponent::GetParticleProperties() const
	{
		return m_partProperties;
	}

	void ParticleEmitterComponent::SetParticleProperties(const ParticleProperties& pp)
	{
		m_partProperties = pp;
	}


	float ParticleEmitterComponent::GetMaxLifetime()const
	{
		return m_partProperties.maxLifetime;
	}

	void ParticleEmitterComponent::SetMaxLifetime(float f)
	{		
		auto dif = m_partProperties.maxLifetime - std::max(f, 0.0f);
		std::for_each(m_persistentData.begin(), m_persistentData.end(),
			[=](ParticlePersistence& pp) { pp.m_maxLifetime = std::max(0.0f, pp.m_maxLifetime - dif); });
		m_partProperties.maxLifetime = std::max(f, 0.0f);
	}
}
