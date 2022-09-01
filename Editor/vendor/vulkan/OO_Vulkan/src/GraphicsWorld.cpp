#include "GraphicsWorld.h"

int32_t GraphicsWorld::CreateObjectInstance()
{
	return CreateObjectInstance(ObjectInstance());
}

int32_t GraphicsWorld::CreateObjectInstance(ObjectInstance obj)
{
	return m_ObjectInstances.Add(obj);
}

ObjectInstance& GraphicsWorld::GetObjectInstance(int32_t id)
{
	return m_ObjectInstances.Get(id);
}
