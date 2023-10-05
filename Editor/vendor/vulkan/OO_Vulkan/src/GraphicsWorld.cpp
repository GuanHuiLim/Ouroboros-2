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
#include "Font.h"
#include "VulkanRenderer.h"

void GraphicsWorld::BeginFrame()
{
	PROFILE_SCOPED();
	auto& vr = *VulkanRenderer::get();
	// TODO: What do you do at the beginning of the frame?
	m_objectsCopy.clear();
	m_objectsCopy.reserve(m_ObjectInstances.size());
	for ( ObjectInstance& src: m_ObjectInstances)
	{
		if (src.isSkinned()) 
		{
			auto& mdl = vr.g_globalModels[src.modelID];
			if (src.bones.empty())
			{
				OO_ASSERT(mdl.skeleton->inverseBindPose.size() && "Src model does not have bones");
				src.bones.resize(mdl.skeleton->inverseBindPose.size());
				for (auto& b : src.bones)
				{
					b = mat4(1.0f);
				}
			}
		}
		m_objectsCopy.emplace_back(src);
	}

	m_EmitterCopy.clear();
	m_EmitterCopy.reserve(m_EmitterInstances.size());
	for (const EmitterInstance& src: m_EmitterInstances)
	{
		m_EmitterCopy.emplace_back(src);
	}

	m_UIcopy.clear();
	m_UIcopy.reserve(m_UIInstances.size());
	for (const UIInstance& src: m_UIInstances)
	{
		m_UIcopy.emplace_back(src);
	}

	m_OmniLightCopy.clear();
	m_OmniLightCopy.reserve(m_OmniLightInstances.size());
	for (const OmniLightInstance& src: m_OmniLightInstances)
	{
		m_OmniLightCopy.emplace_back(src);
	}
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

int32_t GraphicsWorld::CreateUIInstance()
{
	return CreateUIInstance(UIInstance{});
}

int32_t GraphicsWorld::CreateUIInstance(UIInstance obj)
{
	++m_entityCount;
	return m_UIInstances.Add(obj);
}

UIInstance & GraphicsWorld::GetUIInstance(int32_t id)
{
	return m_UIInstances.Get(id);
}

void GraphicsWorld::DestroyUIInstance(int32_t id)
{
	m_UIInstances.Remove(id);
	--m_entityCount;
}

void GraphicsWorld::ClearUIInstances()
{
	m_UIInstances.Clear();
	m_uiCount = 0;
}

int32_t GraphicsWorld::CreateLightInstance()
{
	auto light = OmniLightInstance();
	SetLightEnabled(light,true);
	return CreateLightInstance(light);
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
	return static_cast<bool>(flags & ObjectInstanceFlags::SKINNED);
}

bool ObjectInstance::isShadowEnabled()
{
	return static_cast<bool>(flags & ObjectInstanceFlags::SHADOW_ENABLED);
}

bool ObjectInstance::isShadowCaster()
{
	return static_cast<bool>(flags & ObjectInstanceFlags::SHADOW_CASTER);
}

bool ObjectInstance::isRenderable()
{
	return static_cast<bool>(flags & ObjectInstanceFlags::RENDER_ENABLED);
}

bool ObjectInstance::isDynamic()
{
	return static_cast<bool>(flags & ObjectInstanceFlags::DYNAMIC_INSTANCE);
}

bool ObjectInstance::isTransparent()
{
	return static_cast<bool>(flags & ObjectInstanceFlags::TRANSPARENT);
}

void SetCastsShadows(LocalLightInstance& l, bool s)
{
	l.info.x = s ? 1 : -1;
}

bool GetCastsShadows(const LocalLightInstance& l)
{
	return l.info.x >= 1;
}

void SetCastsShadows(OmniLightInstance& l, bool s)
{
	SetCastsShadows(*reinterpret_cast<LocalLightInstance*>(&l),s);
}

bool GetCastsShadows(const OmniLightInstance& l)
{
	return GetCastsShadows(*reinterpret_cast<const LocalLightInstance*>(&l));
}

void SetCastsShadows(SpotLightInstance& l, bool s)
{
	SetCastsShadows(*reinterpret_cast<LocalLightInstance*>(&l),s);
}

bool GetCastsShadows(const SpotLightInstance& l)
{
	return GetCastsShadows(*reinterpret_cast<const LocalLightInstance*>(&l));
}

void UIInstance::SetText(bool s)
{
	if (s)
	{
		flags = flags | UIInstanceFlags::TEXT_INSTANCE;
	}
	else
	{
		flags = flags & (~UIInstanceFlags::TEXT_INSTANCE);
	}
}

bool UIInstance::isText()
{
	return static_cast<bool>(flags & UIInstanceFlags::TEXT_INSTANCE);
}

void UIInstance::SetScreenSpace(bool s)
{
	if (s)
	{
		flags = flags | UIInstanceFlags::SCREEN_SPACE;
	}
	else
	{
		flags = flags & (~UIInstanceFlags::SCREEN_SPACE);
	}
}

bool UIInstance::isScreenSpace()
{
	return static_cast<bool>(flags & UIInstanceFlags::SCREEN_SPACE);
}

void UIInstance::SetRenderEnabled(bool s)
{
	if (s)
	{
		flags = flags | UIInstanceFlags::RENDER_ENABLED;
	}
	else
	{
		flags = flags & (~UIInstanceFlags::RENDER_ENABLED);
	}
}

bool UIInstance::isRenderable()
{
	return static_cast<bool>(flags & UIInstanceFlags::RENDER_ENABLED);
}
