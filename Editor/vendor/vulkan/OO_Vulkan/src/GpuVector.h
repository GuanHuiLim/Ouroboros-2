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
#include "Profiling.h"
#include "DelayedDeleter.h"
#include "VulkanUtils.h"

struct VulkanDevice;

extern uint64_t accumulatedBytes;

template <typename T>
class GpuVector {
public:
	GpuVector();
	GpuVector(VulkanDevice* device);
	void Init(VkBufferUsageFlags usage, std::string name = {});
	void Init(VulkanDevice* device, VkBufferUsageFlags usage, std::string name = {});

	void blockingWriteTo(size_t size, const void* data, VkQueue queue, VkCommandPool pool, size_t offset = 0);
	void writeToCmd(size_t writeSize, const void* data, VkCommandBuffer command, size_t offset = 0);

	void addWriteCommand(size_t size, const void* data, size_t offset = 0);
	void flushToGPU(VkCommandBuffer command);

	void resize(VkCommandBuffer cmd, size_t size);
	void reserve(VkCommandBuffer cmd, size_t size);
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
	std::string m_name{"UNNAMED_VECTOR"};
	size_t m_size{ 0 };
	size_t m_capacity{ 0 };
	VkBufferUsageFlags m_usage{};
	oGFX::AllocatedBuffer m_buffer{};
	VkDescriptorBufferInfo m_descriptor{};

	VulkanDevice* m_device{ nullptr };

	bool m_mustUpdate;
	std::vector<VkBufferCopy>m_copyRegions;
	std::vector<T>m_cpuBuffer;
};

#ifndef GPU_VECTOR_CPP
#define GPU_VECTOR_CPP
#include "GpuVector.h"
#include "VulkanUtils.h"
#include "VulkanDevice.h"

class VulkanRenderer;
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
void GpuVector<T>::Init(VkBufferUsageFlags usage, std::string name)
{
	assert(m_device != nullptr); // invalid device ptr. or didnt provide
	if (name.empty() == false) m_name = name;
	m_usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VmaAllocatorCreateFlags noflags = 0;
	oGFX::CreateBuffer(m_device->m_allocator, 1, m_usage,
		noflags, m_buffer);
	VK_NAME(m_device->logicalDevice, m_name.c_str(), m_buffer.buffer);
}

template<typename T>
inline void GpuVector<T>::Init(VulkanDevice* device, VkBufferUsageFlags usage, std::string name)
{
	m_device = device;
	Init(usage,std::move(name));
}

template <typename T>
void GpuVector<T>::blockingWriteTo(size_t writeSize,const void* data, VkQueue queue, VkCommandPool pool, size_t offset)
{
	if (writeSize == 0) 
		return;
	PROFILE_SCOPED();
	if ((writeSize + offset) > m_capacity)
	{
		// TODO:  maybe resize some amount instead of perfect amount?
		assert(true);
		resize(m_capacity?m_capacity*2 : 64, queue, pool);
		blockingWriteTo(writeSize, data, queue, pool, offset);
		return;
	}

	
	using namespace oGFX;
	//get writeSize of buffer needed for vertices
	VkDeviceSize bufferBytes = writeSize*sizeof(T);
	VkDeviceSize writeBytesOffset = offset * sizeof(T);

	//temporary buffer to stage vertex data before transferring to GPU
	oGFX::AllocatedBuffer stagingBuffer{};
	
	//create buffer and allocate memory to it
	CreateBuffer(m_device->m_allocator, bufferBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

	//MAP MEMORY TO VERTEX BUFFER
	void *mappedData = nullptr;												
	auto result = vmaMapMemory(m_device->m_allocator, stagingBuffer.alloc, &mappedData);
	if (result != VK_SUCCESS)
	{
		assert(false);
	}
	memcpy(mappedData, data, (size_t)bufferBytes);					
	vmaUnmapMemory(m_device->m_allocator, stagingBuffer.alloc);					

	CopyBuffer(m_device->logicalDevice, queue,pool,
		stagingBuffer.buffer, m_buffer.buffer, bufferBytes, writeBytesOffset);


	{
		PROFILE_SCOPED("Clean buffer");

		//clean up staging buffer parts
		vmaDestroyBuffer(m_device->m_allocator, stagingBuffer.buffer, stagingBuffer.alloc);
	}

	//not sure what to do here, we just assume that its tightly packed
	if (offset < m_size)
	{
		m_size -= m_size - offset;
	}
	m_size += writeSize;

}

template <typename T>
void GpuVector<T>::writeToCmd(size_t writeSize, const void* data,VkCommandBuffer command, size_t offset)
{
	if (writeSize == 0)
		return;
	PROFILE_SCOPED();
	if ((writeSize + offset) > m_capacity)
	{
		// TODO:  maybe resize some amount instead of perfect amount?
		assert(true);
		resize(command, m_capacity ? m_capacity * 2 : 64);
		writeToCmd(writeSize, data, command, offset);
		return;
	}


	using namespace oGFX;
	//get writeSize of buffer needed for vertices
	VkDeviceSize bufferBytes = writeSize * sizeof(T);
	VkDeviceSize writeBytesOffset = offset * sizeof(T);

	//temporary buffer to stage vertex data before transferring to GPU
	oGFX::AllocatedBuffer stagingBuffer{};

	//create buffer and allocate memory to it
	CreateBuffer(m_device->m_allocator, bufferBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

	//MAP MEMORY TO VERTEX BUFFER
	void* mappedData = nullptr;
	auto result = vmaMapMemory(m_device->m_allocator, stagingBuffer.alloc, &mappedData);
	if (result != VK_SUCCESS)
	{
		assert(false);
	}
	memcpy(mappedData, data, (size_t)bufferBytes);
	vmaUnmapMemory(m_device->m_allocator, stagingBuffer.alloc);

	
	auto commandBuffer = command;
	
	// region of data to copy from and to
	VkBufferCopy bufferCopyRegion{};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = bufferBytes;
	
	// command to copy src buffer to dst buffer
	vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer, m_buffer.buffer, 1, &bufferCopyRegion);
	
	auto fun = [oldBuffer = stagingBuffer, alloc = m_device->m_allocator]() {
		PROFILE_SCOPED("Clean buffer")
			//clean up staging buffer parts
			vmaDestroyBuffer(alloc, oldBuffer.buffer, oldBuffer.alloc);
	};
	DelayedDeleter::get()->DeleteAfterFrames(fun);

	//not sure what to do here, we just assume that its tightly packed
	if (offset < m_size)
	{
		m_size -= m_size - offset;
	}
	m_size += writeSize;

}

template<typename T>
inline void GpuVector<T>::addWriteCommand(size_t writeSize, const void* data, size_t offset)
{
	VkDeviceSize dataBytes = writeSize * sizeof(T);
	VkDeviceSize writeBytesOffset = offset * sizeof(T);

	auto oldSize = m_cpuBuffer.size();
	VkDeviceSize cpuBytesOffset = oldSize * sizeof(T);

	VkBufferCopy bufferCopyRegion{};
	bufferCopyRegion.srcOffset = cpuBytesOffset;
	bufferCopyRegion.dstOffset = writeBytesOffset;
	bufferCopyRegion.size = dataBytes;

	m_copyRegions.push_back(bufferCopyRegion);
	
	m_cpuBuffer.resize(oldSize + writeSize);
	memcpy(m_cpuBuffer.data() + oldSize, data, dataBytes);

	m_mustUpdate = true;

}

template<typename T>
inline void GpuVector<T>::flushToGPU(VkCommandBuffer command)
{
	VkDeviceSize totalDataSize{};
	size_t largestWrite{};
	for (const auto& copycmd: m_copyRegions)
	{
		totalDataSize += copycmd.size;
		largestWrite = std::max(largestWrite, copycmd.dstOffset + copycmd.size);
	}
	size_t maxElement = largestWrite / sizeof(T);

	if (totalDataSize == 0)
	{
		return;
	}

	PROFILE_SCOPED();
	if ((maxElement) > m_capacity)
	{
		assert(true);
		resize(command, maxElement);
	}

	//temporary buffer to stage vertex data before transferring to GPU
	oGFX::AllocatedBuffer stagingBuffer;

	//create buffer and allocate memory to it
	oGFX::CreateBuffer(m_device->m_allocator, totalDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

	//MAP MEMORY TO VERTEX BUFFER
	void* mappedData = nullptr;
	auto result = vmaMapMemory(m_device->m_allocator, stagingBuffer.alloc, &mappedData);
	if (result != VK_SUCCESS)
	{
		assert(false);
	}
	memcpy(mappedData, m_cpuBuffer.data(), (size_t)totalDataSize);
	vmaUnmapMemory(m_device->m_allocator, stagingBuffer.alloc);
	
	//m_cpuBuffer.clear(); // good for small memory..
	m_cpuBuffer = {}; // release the memory because it could be quite big

	auto commandBuffer = command;

	// command to copy src buffer to dst buffer
	vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer, m_buffer.buffer, (uint32_t)m_copyRegions.size(), m_copyRegions.data());
	m_copyRegions.clear();

	auto fun = [oldBuffer = stagingBuffer, alloc = m_device->m_allocator]() {
		PROFILE_SCOPED("Clean buffer");
		//clean up staging buffer parts
		vmaDestroyBuffer(alloc, oldBuffer.buffer, oldBuffer.alloc);
	};
	DelayedDeleter::get()->DeleteAfterFrames(fun);

	m_mustUpdate = false;
}

template <typename T>
void GpuVector<T>::resize(VkCommandBuffer cmd, size_t size)
{
	std::cout << "[GpuVector<T>::resize] " << "Resizing from " << m_size << " to " << size << "\n";
	reserve(cmd, size);
	m_size = size;	
}

template <typename T>
void GpuVector<T>::reserve(VkCommandBuffer cmd, size_t size)
{
	if (size <= m_capacity) return;
	PROFILE_SCOPED();

	using namespace oGFX;
	VkDeviceSize bufferSize = size * sizeof(T);

	if (bufferSize == 0) return;

	oGFX::AllocatedBuffer tempBuffer;
	
	VmaPoolCreateFlags noflags = 0;
	CreateBuffer(m_device->m_allocator, bufferSize, m_usage,
		noflags , tempBuffer); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT make this buffer local to the GPU
																		//copy staging buffer to vertex buffer on GPU

	if (m_size != 0)
	{
		CopyBuffer(cmd, m_buffer.buffer, tempBuffer.buffer, m_size * sizeof(T));
	}

	auto fun = [oldBuffer = m_buffer, alloc = m_device->m_allocator]() {
			PROFILE_SCOPED("Clean up buffer");
			vmaDestroyBuffer(alloc, oldBuffer.buffer, oldBuffer.alloc);
		};
	DelayedDeleter::get()->DeleteAfterFrames(fun);


	m_buffer = tempBuffer;
	VK_NAME(m_device->logicalDevice, m_name.c_str(), m_buffer.buffer);

	// accumulate bytes
	accumulatedBytes -= m_capacity;
	accumulatedBytes += size;

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
	return m_buffer.buffer;
}
template <typename T>
const VkBuffer* GpuVector<T>::getBufferPtr() const
{
	return &m_buffer.buffer;
}
template <typename T>
void GpuVector<T>::destroy()
{
	//clean up old buffer
	if (m_buffer.buffer)
	{
		vmaDestroyBuffer(m_device->m_allocator, m_buffer.buffer, m_buffer.alloc);
		m_buffer.buffer = VK_NULL_HANDLE;
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
	m_descriptor.buffer = m_buffer.buffer;
	m_descriptor.offset = 0;
	m_descriptor.range = VK_WHOLE_SIZE;
	return m_descriptor;
}

template<typename T>
inline const VkDescriptorBufferInfo* GpuVector<T>::GetBufferInfoPtr()
{
	m_descriptor.buffer = m_buffer.buffer;
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

