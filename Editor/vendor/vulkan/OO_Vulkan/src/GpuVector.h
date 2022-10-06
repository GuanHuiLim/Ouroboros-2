/************************************************************************************//*!
\file           GpuVector.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a wrapper object for resiable GPU buffer. Generally bad idea

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "vulkan/vulkan.h"
#include <iostream>

struct VulkanDevice;

template <typename T>
class GpuVector{
public:
	GpuVector();
	GpuVector(VulkanDevice* device);
	void Init(VkBufferUsageFlags usage);
	void Init(VulkanDevice* device,VkBufferUsageFlags usage);

	void writeTo(size_t size, void* data, size_t offset = 0);

	void resize(size_t size);
	void reserve(size_t size);
	size_t size() const;

	VkBuffer getBuffer()const;
	const VkBuffer* getBufferPtr()const;

	void destroy();
	void clear();

	const VkDescriptorBufferInfo& GetDescriptorBufferInfo();
	const VkDescriptorBufferInfo* GetBufferInfoPtr();
	bool MustUpdate();
	void Updated();

public:
	size_t m_size{0};
	size_t m_capacity{0};
	VkBufferUsageFlags m_usage{};
	VkBuffer m_buffer{VK_NULL_HANDLE};
	VkDeviceMemory m_gpuMemory{VK_NULL_HANDLE};
	VkDescriptorBufferInfo m_descriptor{};

	VulkanDevice* m_device{nullptr};

	bool m_mustUpdate;
};


#ifndef GPU_VECTOR_CPP
#define GPU_VECTOR_CPP
#include "GpuVector.h"
#include "VulkanUtils.h"
#include "VulkanDevice.h"

template<typename T>
GpuVector<T>::GpuVector() : 
	m_device{nullptr},
	m_buffer{VK_NULL_HANDLE}
{

}

template <typename T>
GpuVector<T>::GpuVector(VulkanDevice* device) :
	m_device{ device },
	m_buffer{VK_NULL_HANDLE}
{

}

template <typename T>
void GpuVector<T>::Init(VkBufferUsageFlags usage)
{
	assert(m_device != nullptr); // invalid device ptr. or didnt provide
	assert(m_buffer == VK_NULL_HANDLE); // called init twice
	assert(m_gpuMemory == VK_NULL_HANDLE); // called init twice
	m_usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	oGFX::CreateBuffer(m_device->physicalDevice, m_device->logicalDevice, 1, m_usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_buffer, &m_gpuMemory);
}

template<typename T>
inline void GpuVector<T>::Init(VulkanDevice* device, VkBufferUsageFlags usage)
{
	m_device = device;
	Init(usage);
}

template <typename T>
void GpuVector<T>::writeTo(size_t writeSize, void* data, size_t offset)
{
	if ((writeSize + offset) > m_capacity)
	{
		// TODO:  maybe resize some amount instead of perfect amount?
		assert(true);
		resize(m_capacity?m_capacity*2 : 64);
		writeTo(writeSize, data, offset);
		return;
	}

	if (writeSize == 0) 
		return;
	
	using namespace oGFX;
	//get writeSize of buffer needed for vertices
	VkDeviceSize bufferBytes = writeSize*sizeof(T);
	VkDeviceSize writeBytesOffset = offset * sizeof(T);

	//temporary buffer to stage vertex data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory; 

	//create buffer and allocate memory to it
	CreateBuffer(m_device->physicalDevice, m_device->logicalDevice, bufferBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	//MAP MEMORY TO VERTEX BUFFER
	void *mappedData = nullptr;												
	vkMapMemory(m_device->logicalDevice, stagingBufferMemory, 0, bufferBytes, 0, &mappedData);	
	memcpy(mappedData, data, (size_t)bufferBytes);					
	vkUnmapMemory(m_device->logicalDevice, stagingBufferMemory);					

	CopyBuffer(m_device->logicalDevice, m_device->graphicsQueue, m_device->commandPool,
		stagingBuffer, m_buffer, bufferBytes, writeBytesOffset);

	//clean up staging buffer parts
	vkDestroyBuffer(m_device->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(m_device->logicalDevice, stagingBufferMemory, nullptr);

	//not sure what to do here, we just assume that its tightly packed
	if (offset < m_size)
	{
		m_size -= m_size - offset;
	}
	m_size += writeSize;

}

template <typename T>
void GpuVector<T>::resize(size_t size)
{
	std::cout << "[GpuVector<T>::resize] " << "Resizing from " << m_size << " to " << size << "\n";
	reserve(size);
	m_size = size;	
}

template <typename T>
void GpuVector<T>::reserve(size_t size)
{

	if (size < m_capacity) return;

	using namespace oGFX;
	VkDeviceSize bufferSize = size * sizeof(T);

	if (bufferSize == 0) return;

	VkBuffer tempBuffer;
	VkDeviceMemory tempMemory;
	CreateBuffer(m_device->physicalDevice, m_device->logicalDevice, bufferSize, m_usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &tempBuffer, &tempMemory); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT make this buffer local to the GPU
																		//copy staging buffer to vertex buffer on GPU

	if (m_size != 0)
	{
		CopyBuffer(m_device->logicalDevice, m_device->graphicsQueue, m_device->commandPool, m_buffer, tempBuffer, m_size* sizeof(T));
	}

	//clean up old buffer
	vkDestroyBuffer(m_device->logicalDevice, m_buffer, nullptr);
	vkFreeMemory(m_device->logicalDevice, m_gpuMemory, nullptr);

	m_buffer = tempBuffer;
	m_gpuMemory = tempMemory;

	m_capacity = size;

	m_mustUpdate = true;
}

template <typename T>
size_t GpuVector<T>::size() const
{
	return m_size;
}
template <typename T>
VkBuffer GpuVector<T>::getBuffer() const
{
	return m_buffer;
}
template <typename T>
const VkBuffer* GpuVector<T>::getBufferPtr() const
{
	return &m_buffer;
}
template <typename T>
void GpuVector<T>::destroy()
{
	//clean up old buffer
	if (m_buffer)
	{
		vkDestroyBuffer(m_device->logicalDevice, m_buffer, nullptr);
		vkFreeMemory(m_device->logicalDevice, m_gpuMemory, nullptr);
	}
}
template <typename T>
void GpuVector<T>::clear()
{
	m_size = 0;
}

template<typename T>
inline const VkDescriptorBufferInfo& GpuVector<T>::GetDescriptorBufferInfo()
{
	m_descriptor.buffer = m_buffer;
	m_descriptor.offset = 0;
	m_descriptor.range = VK_WHOLE_SIZE;
	return m_descriptor;
}

template<typename T>
inline const VkDescriptorBufferInfo* GpuVector<T>::GetBufferInfoPtr()
{
	m_descriptor.buffer = m_buffer;
	m_descriptor.offset = 0;
	m_descriptor.range = VK_WHOLE_SIZE;
	return &m_descriptor;
}

template<typename T>
inline bool GpuVector<T>::MustUpdate()
{
	return m_mustUpdate;
}

template<typename T>
inline void GpuVector<T>::Updated()
{
	m_mustUpdate = false;
}

#endif // !GPU_VECTOR_CPP

