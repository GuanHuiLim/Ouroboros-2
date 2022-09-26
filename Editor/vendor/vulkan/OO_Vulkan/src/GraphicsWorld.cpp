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