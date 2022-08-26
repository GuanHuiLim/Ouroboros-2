#include "Mesh.h"

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<oGFX::Vertex>* vertices, std::vector<uint32_t>* indices, int newTexId)
{
	vertexCount = static_cast<int>(vertices->size());
	indexCount = static_cast<int>(indices->size());
	physicalDevice = newPhysicalDevice;
	device = newDevice;
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
	return vertexBuffer;
}

int Mesh::getIndexCount()
{
	return indexCount;
}

VkBuffer Mesh::getIndexBuffer()
{
	return indexBuffer;
}

void Mesh::destroyBuffers()
{
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
}

void Mesh::CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<oGFX::Vertex>* vertices)
{
	using namespace oGFX;
	//get size of buffer needed for vertices
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

	//temporary buffer to stage vertex data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory; 


	//create buffer and allocate memory to it
	CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	//MAP MEMORY TO VERTEX BUFFER
	void *data = nullptr;												//1. create a pointer to a point in normal memory
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);	//2. map the vertex buffer to that point
	memcpy(data, vertices->data(), (size_t)bufferSize);					//3. copy memory from vertices vector to the point
	vkUnmapMemory(device, stagingBufferMemory);							//4. unmap the vertex buffer memory

																		//create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
																		// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by the GPU and not the CPU (host)
	CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT make this buffer local to the GPU

																				  //copy staging buffer to vertex buffer on GPU
	CopyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	//clean up staging buffer parts
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Mesh::CreateIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
	using namespace oGFX;
	//get size of buffer needed for indices
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

	//temporary buffer to stage index data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	//MAP MEMORY TO INDEX BUFFER
	void *data = nullptr;												
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);	
	memcpy(data, indices->data(), (size_t)bufferSize);					
	vkUnmapMemory(device, stagingBufferMemory);	

	// Create buffer for INDEX data on GPU access only area
	CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

	// Copy from staging buffer to GPU access buffer
	CopyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, indexBuffer, bufferSize);

	//destroy + release staging buffer resources
	vkDestroyBuffer(device, stagingBuffer, nullptr); 
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}


