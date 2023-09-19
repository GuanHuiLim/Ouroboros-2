/************************************************************************************//*!
\file           Mesh.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief               Defines a mesh object for used with indexed draw (OLD)

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "Mesh.h"

Mesh::Mesh(VkDevice newDevice, VmaAllocator newAlloc, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<oGFX::Vertex>* vertices, std::vector<uint32_t>* indices, int newTexId)
{
	vertexCount = static_cast<int>(vertices->size());
	indexCount = static_cast<int>(indices->size());
	device = newDevice;
	vmaAllocator = newAlloc;
	CreateVertexBuffer(transferQueue,transferCommandPool,vertices);
	CreateIndexBuffer(transferQueue, transferCommandPool, indices);
	model = glm::mat4(1.0f);
	texId = newTexId;
}

Mesh::~Mesh()
{
}

void Mesh::SetTransform(glm::mat4 newModel)
{
	model = newModel;
}

const glm::mat4& Mesh::GetTransform()
{
	return model;
}

int Mesh::getTexId()
{
	return texId;
}

int Mesh::getVertexCount()
{
	return vertexCount;
}

VkBuffer Mesh::getVertexBuffer()
{
	return vertexBuffer.buffer;
}

int Mesh::getIndexCount()
{
	return indexCount;
}

VkBuffer Mesh::getIndexBuffer()
{
	return indexBuffer.buffer;
}

void Mesh::destroyBuffers()
{
	vmaDestroyBuffer(vmaAllocator, vertexBuffer.buffer, vertexBuffer.alloc);
	vmaDestroyBuffer(vmaAllocator, indexBuffer.buffer, indexBuffer.alloc);
}

void Mesh::CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<oGFX::Vertex>* vertices)
{
	using namespace oGFX;
	//get size of buffer needed for vertices
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

	//temporary buffer to stage vertex data before transferring to GPU
	oGFX::AllocatedBuffer stagingBuffer{};
	
	//create buffer and allocate memory to it
	CreateBuffer(vmaAllocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

	//MAP MEMORY TO VERTEX BUFFER
	void *data = nullptr;												//1. create a pointer to a point in normal memory
	vmaMapMemory(vmaAllocator, stagingBuffer.alloc, &data);	//2. map the vertex buffer to that point
	memcpy(data, vertices->data(), (size_t)bufferSize);					//3. copy memory from vertices vector to the point
	vmaUnmapMemory(vmaAllocator, stagingBuffer.alloc);							//4. unmap the vertex buffer memory

																		//create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
																		// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by the GPU and not the CPU (host)
	VmaAllocationCreateFlags noflags = 0;
	CreateBuffer(vmaAllocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		noflags, vertexBuffer); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT make this buffer local to the GPU

																				  //copy staging buffer to vertex buffer on GPU
	CopyBuffer(device, transferQueue, transferCommandPool, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

	//clean up staging buffer parts
	vmaDestroyBuffer(vmaAllocator, stagingBuffer.buffer, stagingBuffer.alloc);
}

void Mesh::CreateIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
	using namespace oGFX;
	//get size of buffer needed for indices
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

	//temporary buffer to stage index data before transferring to GPU
	oGFX::AllocatedBuffer stagingBuffer{};

	CreateBuffer(vmaAllocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

	//MAP MEMORY TO INDEX BUFFER
	void *data = nullptr;												
	vmaMapMemory(vmaAllocator,stagingBuffer.alloc,&data);	
	memcpy(data, indices->data(), (size_t)bufferSize);					
	vmaUnmapMemory(vmaAllocator, stagingBuffer.alloc);	

	// Create buffer for INDEX data on GPU access only area
	VmaAllocationCreateFlags noflags = 0;
	CreateBuffer(vmaAllocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		noflags, indexBuffer);

	// Copy from staging buffer to GPU access buffer
	CopyBuffer(device, transferQueue, transferCommandPool, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

	//destroy + release staging buffer resources
	vmaDestroyBuffer(vmaAllocator, stagingBuffer.buffer, stagingBuffer.alloc);
}


