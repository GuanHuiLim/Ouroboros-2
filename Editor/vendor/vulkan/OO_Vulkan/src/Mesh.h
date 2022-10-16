/************************************************************************************//*!
\file           Mesh.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a mesh object for used with indexed draw (OLD)

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <vulkan/vulkan.h>
#include "MathCommon.h"

#pragma warning( push )
#pragma warning( disable : 26451 ) // vendor overflow
#include "assimp/scene.h"
#pragma warning( pop )

#include "VulkanUtils.h"
#include "Node.h"

#include <vector>

namespace oGFX
{
	struct Mesh
	{
		uint32_t vertexOffset{};
		uint32_t vertexCount{};
		uint32_t indicesOffset{};
		uint32_t indicesCount{};
		uint32_t textureIndex{};
	};
}

class Mesh
{
public:
	Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice,
		VkQueue transferQueue,VkCommandPool transferCommandPool,
		std::vector<oGFX::Vertex> *vertices, std::vector<uint32_t> *indices, int newTexId);

	~Mesh();

	void SetTransform(glm::mat4 newModel);
	const glm::mat4& GetTransform();

	int getTexId();

	int getVertexCount();
	VkBuffer getVertexBuffer();

	int getIndexCount();
	VkBuffer getIndexBuffer();

	void destroyBuffers();


private:

	glm::mat4 model{ 1.0f };
	int texId;

	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	int indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	void CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<oGFX::Vertex> *vertices);
	void CreateIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t> *indices);

	
};

