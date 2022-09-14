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
	++entityCount;
	return m_ObjectInstances.Add(obj);
}

ObjectInstance& GraphicsWorld::GetObjectInstance(int32_t id)
{
	return m_ObjectInstances.Get(id);
}

void GraphicsWorld::DestroyObjectInstance(int32_t id)
{
	m_ObjectInstances.Remove(id);
	--entityCount;
}

void GraphicsWorld::ClearObjectInstances()
{
	m_ObjectInstances.Clear();
	entityCount = 0;
}
