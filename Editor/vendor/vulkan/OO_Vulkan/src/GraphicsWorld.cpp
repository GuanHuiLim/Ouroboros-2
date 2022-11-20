#include "GraphicsWorld.h"
#include "GraphicsWorld.h"
#include "GraphicsWorld.h"
#include "GraphicsWorld.h"
#include "GraphicsWorld.h"
/************************************************************************************//*!
\file           GraphicsWorld.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines graphics world, a wrapper for objects that require to be rendered.
    This is used as the main tnerface between the renderer and external engine

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "GraphicsWorld.h"

void GraphicsWorld::BeginFrame()
{
	// TODO: What do you do at the beginning of the frame?
}

void GraphicsWorld::EndFrame()
{
	// TODO: What do you do at the end of the frame?
}

int32_t GraphicsWorld::CreateObjectInstance()
{
	return CreateObjectInstance(ObjectInstance());
}

int32_t GraphicsWorld::CreateObjectInstance(ObjectInstance obj)
{
	++m_entityCount;
	return m_ObjectInstances.Add(obj);
}

ObjectInstance& GraphicsWorld::GetObjectInstance(int32_t id)
{
	return m_ObjectInstances.Get(id);
}

void GraphicsWorld::DestroyObjectInstance(int32_t id)
{
	m_ObjectInstances.Remove(id);
	--m_entityCount;
}

void GraphicsWorld::ClearObjectInstances()
{
	m_ObjectInstances.Clear();
	m_entityCount = 0;
}


int32_t GraphicsWorld::CreateLightInstance()
{
	return CreateLightInstance(OmniLightInstance());
}

int32_t GraphicsWorld::CreateLightInstance(OmniLightInstance obj)
{
	++m_lightCount;
	return m_OmniLightInstances.Add(obj);
}

OmniLightInstance& GraphicsWorld::GetLightInstance(int32_t id)
{
	return m_OmniLightInstances.Get(id);
}

void GraphicsWorld::DestroyLightInstance(int32_t id)
{
	m_OmniLightInstances.Remove(id);
	--m_lightCount;
}

void GraphicsWorld::ClearLightInstances()
{
	m_OmniLightInstances.Clear();
	m_lightCount = 0;
}

int32_t GraphicsWorld::CreateEmitterInstance()
{
	return CreateEmitterInstance(EmitterInstance());
}

int32_t GraphicsWorld::CreateEmitterInstance(EmitterInstance obj)
{
	++m_emitterCount;
	return m_EmitterInstances.Add(obj);
}

EmitterInstance& GraphicsWorld::GetEmitterInstance(int32_t id)
{
	return m_EmitterInstances.Get(id);
}

void GraphicsWorld::DestroyEmitterInstance(int32_t id)
{
	m_EmitterInstances.Remove(id);
	--m_emitterCount;
}

void GraphicsWorld::ClearEmitterInstances()
{
	m_EmitterInstances.Clear();
	m_emitterCount = 0;
}

void GraphicsWorld::SubmitParticles(std::vector<ParticleData>& particleData, uint32_t cnt, int32_t eID)
{
	if (cnt == 0) return;

	auto& emitter = GetEmitterInstance(eID);

	emitter.particles.resize(cnt);
	
	memcpy(emitter.particles.data(), particleData.data(), cnt * sizeof(ParticleData));

}

void ObjectInstance::SetShadowCaster(bool s)
{
	if (s)
	{
		flags = flags | ObjectInstanceFlags::SHADOW_CASTER;
	}
	else
	{
		flags = flags& (~ObjectInstanceFlags::SHADOW_CASTER);
	}
}

void ObjectInstance::SetShadowReciever(bool s)
{
	if (s)
	{
		flags = flags | ObjectInstanceFlags::SHADOW_RECEIVER;
	}
	else
	{
		flags = flags& (~ObjectInstanceFlags::SHADOW_RECEIVER);
	}
}

void ObjectInstance::SetSkinned(bool s)
{
	if (s)
	{
		flags = flags | ObjectInstanceFlags::SKINNED;
	}
	else
	{
		flags = flags& (~ObjectInstanceFlags::SKINNED);
	}
}

void ObjectInstance::SetShadowEnabled(bool s)
{
	if (s)
	{
		flags = flags | ObjectInstanceFlags::SHADOW_ENABLED;
	}
	else
	{
		flags = flags& (~ObjectInstanceFlags::SHADOW_ENABLED);
	}
}

void ObjectInstance::SetRenderEnabled(bool s)
{
	if (s)
	{
		flags = flags | ObjectInstanceFlags::RENDER_ENABLED;
	}
	else
	{
		flags = flags& (~ObjectInstanceFlags::RENDER_ENABLED);
	}
}

bool ObjectInstance::isSkinned()
{
	return flags & ObjectInstanceFlags::SKINNED;
}

bool ObjectInstance::isShadowEnabled()
{
	return flags & ObjectInstanceFlags::SHADOW_ENABLED;
}

bool ObjectInstance::isRenderable()
{
	return flags & ObjectInstanceFlags::RENDER_ENABLED;
}

void SetCastsShadows(LocalLightInstance& l, bool s)
{
	l.info.x = s ? -1 : 1;
}

bool GetCastsShadows(LocalLightInstance& l)
{
	return l.info.x == -1;
}

void SetCastsShadows(OmniLightInstance& l, bool s)
{
	SetCastsShadows(*reinterpret_cast<LocalLightInstance*>(&l),s);
}

bool GetCastsShadows(OmniLightInstance& l)
{
	return GetCastsShadows(*reinterpret_cast<LocalLightInstance*>(&l));
}

void SetCastsShadows(SpotLightInstance& l, bool s)
{
	SetCastsShadows(*reinterpret_cast<LocalLightInstance*>(&l),s);
}

bool GetCastsShadows(SpotLightInstance& l)
{
	return GetCastsShadows(*reinterpret_cast<LocalLightInstance*>(&l));
}
