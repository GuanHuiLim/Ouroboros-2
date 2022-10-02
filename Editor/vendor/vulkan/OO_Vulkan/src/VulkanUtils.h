/************************************************************************************//*!
\file           VulkanUtils.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief          Utility functiosn for vulkan ease of use. mostly inlined

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <vulkan/vulkan.h>
#include "MathCommon.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <typeindex>

#include "../shaders/shared_structs.h"


namespace oGFX::vkutils::tools
{
	std::string VkResultString(VkResult value);
	std::string VkFormatString(VkFormat value);
	std::string VkColorSpaceKHRString(VkColorSpaceKHR value);
};

#ifndef MESSAGE_BOX_ONCE
// Use this to catch potential problems, especially since default assert is ignored in Release mode.
#define MESSAGE_BOX_ONCE(winhdl, msg, title) \
    do                                       \
    {                                        \
        static bool once = false;            \
        if (!once)                           \
        {                                    \
            MessageBoxW(winhdl, (LPCWSTR)msg, (LPCWSTR)title, MB_ICONWARNING | MB_OK); \
            once = true;                     \
        }                                    \
    } while (0)
#endif // !MESSAGE_BOX_ONCE

#ifndef VK_CHK
#define VK_CHK(x) \
	do{\
	VkResult result = x;\
	if(result != VK_SUCCESS)\
	{\
		std::cout << oGFX::vkutils::tools::VkResultString(result) << std::endl;\
		assert(result == VK_SUCCESS);\
		throw std::runtime_error("Failed Vulkan Check");\
	}\
}while(0)
#endif // !VK_CHK


VkDebugReportObjectTypeEXT GetDebugNameExtTypeByID(std::type_index id);


namespace oGFX
{
	void SetVulkanObjectName(VkDevice device, const VkDebugMarkerObjectNameInfoEXT& info);
}


#ifndef VK_NAME

#ifdef _DEBUG



#define VK_NAME(DEVICE, NAME, OBJ) do{\
VkDebugMarkerObjectNameInfoEXT nameInfo = {};\
nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;\
auto objType = std::type_index(typeid(decltype(OBJ)));\
auto eType = GetDebugNameExtTypeByID(objType);\
nameInfo.objectType = eType;\
nameInfo.object = (uint64_t)OBJ;\
nameInfo.pObjectName = NAME;\
oGFX::SetVulkanObjectName(DEVICE,nameInfo);\
}while(0)
#else
#define VK_NAME(DEVICE, NAME, OBJ)   
#endif // DEBUG

#endif



struct VulkanInstance;
struct VulkanDevice;
namespace oGFX
{

	glm::mat4 customOrtho(float aspect_ratio, float size, float nr, float fr);

	// Indices (locations) of Queue Familities (if they exist)
	struct QueueFamilyIndices
	{
		int graphicsFamily = -1; //location of graphics queue family //as per vulkan standard, if we have a graphics family, we have a transfer family
		int presentationFamily = -1;

		//check if queue familities are valid
		bool isValid()
		{
			return graphicsFamily >= 0 && presentationFamily >= 0;
		}
	};


	struct SwapChainDetails
	{
		//surfaces properties , image sizes/extents etc...
		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		//surface image formats, eg. RGBA, data size of each color
		std::vector<VkSurfaceFormatKHR> formats;
		//how images should be presentated to screen, filo fifo etc..
		std::vector<VkPresentModeKHR> presentationModes;
		SwapChainDetails() :surfaceCapabilities{} {}
	};

	struct Vertex
	{
		//float pos[3] ; // Vertex position (x, y, z)
		//float col[3] ; // Vertex colour (r, g, b)
		//float tex[2] ; // Texture Coords(u,v)
		glm::vec3 pos{0.0f}; // Vertex position (x, y, z)
		glm::vec3 norm{}; // Vertex normal (x, y, z)
		glm::vec3 col{0.0f,1.0f,0.0f}; // Vertex colour (r, g, b)
		glm::vec2 tex{}; // Texture Coords(u,v)
		glm::vec3 tangent{}; // Vertex normal (x, y, z)
		uint32_t boneWeights{};
	};

	struct DebugVertex
	{
		glm::vec3 pos{ 0.0f }; // Vertex position (x, y, z)
		glm::vec3 col{ 0.0f,1.0f,0.0f }; // Vertex colour (r, g, b)
	};

	// Per-instance data block
	struct InstanceData {
		uvec4 instanceAttributes{}; // ID, material, ...

		/* // this is before trying to combine	
		glm::mat4 matrix;
		uint32_t albedo;
		uint32_t normal;
		uint32_t occlusion;
		uint32_t roughness;
		*/
	};

	const std::vector<VkVertexInputBindingDescription>& GetGFXVertexInputBindings();	
	const std::vector<VkVertexInputAttributeDescription>& GetGFXVertexInputAttributes();

	oGFX::SwapChainDetails GetSwapchainDetails(VulkanInstance& instance, VkPhysicalDevice device);
	oGFX::QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	VkImageView CreateImageView(VulkanDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	VkFormat ChooseSupportedFormat(VulkanDevice& device, const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

	VkImage CreateImage(VulkanDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory* imageMemory);

	uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties);

	VkShaderModule CreateShaderModule(VulkanDevice& device, const std::vector<char>& code);

	std::vector<char> readFile(const std::string& filename);

	void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize,
		VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags bufferProperties,
		VkBuffer* buffer, VkDeviceMemory* bufferMemory);

	void CopyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool,
		VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkDeviceSize dstOffset = 0, VkDeviceSize srcOffset = 0);

	VkCommandBuffer beginCommandBuffer(VkDevice device, VkCommandPool commandPool);

	void endAndSubmitCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);

	void TransitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool,
		VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

	void CopyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool,
		VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height);

	unsigned char* LoadTextureFromFile(const std::string& fileName, int& width, int& height, uint64_t& imageSize);
	void FreeTextureFile(uint8_t* data);

	struct FileImageData
	{
		std::string name;
		int32_t w{};
		int32_t h{};
		int32_t channels{};
		uint64_t dataSize{};
		std::vector<uint8_t> imgData{};
		std::vector<VkBufferImageCopy> mipInformation{};
		enum class ExtensionType : uint8_t
		{
			DDS,
			STB
		}decodeType{};

		enum class ImageType : uint8_t
		{
			SRGB,
			LINEAR
		}imgType{ImageType::LINEAR};

		VkFormat format{ VK_FORMAT_R8G8B8A8_UNORM };

		bool Create(const std::string& fileName);
		void Free();
	};

	bool IsFileDDS(const std::string& fileName);

	namespace vkutils
	{
		namespace tools
		{
			void setImageLayout(
				VkCommandBuffer cmdbuffer,
				VkImage image,
				VkImageLayout oldImageLayout,
				VkImageLayout newImageLayout,
				VkImageSubresourceRange subresourceRange,
				VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

			// Uses a fixed sub resource layout with first mip level and layer
			void setImageLayout(
				VkCommandBuffer cmdbuffer,
				VkImage image,
				VkImageAspectFlags aspectMask,
				VkImageLayout oldImageLayout,
				VkImageLayout newImageLayout,
				VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		}

		namespace inits
		{

			inline VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
			{
				VkDescriptorImageInfo descriptorImageInfo {};
				descriptorImageInfo.sampler = sampler;
				descriptorImageInfo.imageView = imageView;
				descriptorImageInfo.imageLayout = imageLayout;
				return descriptorImageInfo;
			}

			inline VkSamplerCreateInfo samplerCreateInfo()
			{
				VkSamplerCreateInfo samplerCreateInfo {};
				samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerCreateInfo.maxAnisotropy = 1.0f;
				return samplerCreateInfo;
			}

			inline VkMemoryAllocateInfo memoryAllocateInfo()
			{
				VkMemoryAllocateInfo memAllocInfo{};
				memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				return memAllocInfo;
			}

			inline VkImageViewCreateInfo imageViewCreateInfo()
			{
				VkImageViewCreateInfo viewInfo{};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				return viewInfo;
			}

			inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
				VkCommandPool commandPool,
				VkCommandBufferLevel level,
				uint32_t bufferCount)
			{
				VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
				commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				commandBufferAllocateInfo.commandPool = commandPool;
				commandBufferAllocateInfo.level = level;
				commandBufferAllocateInfo.commandBufferCount = bufferCount;
				return commandBufferAllocateInfo;
			}

			inline VkSubmitInfo submitInfo()
			{
				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				return submitInfo;
			}

			inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0)
			{
				VkFenceCreateInfo fenceCreateInfo{};
				fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceCreateInfo.flags = flags;
				return fenceCreateInfo;
			}


			inline VkBufferCreateInfo bufferCreateInfo()
			{
				VkBufferCreateInfo bufCreateInfo{};
				bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				return bufCreateInfo;
			}

			inline VkBufferCreateInfo bufferCreateInfo(
				VkBufferUsageFlags usage,
				VkDeviceSize size)
			{
				VkBufferCreateInfo bufCreateInfo{};
				bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufCreateInfo.usage = usage;// size of buffer (size of 1 vertex pos * number of verts)
				bufCreateInfo.size = size;	//multiple types of buffer possible
				bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// similar to swapchain images , we can share vertex buffers
				return bufCreateInfo;
			}

			inline VkImageCreateInfo imageCreateInfo()
			{
				VkImageCreateInfo imageCreateInfo{};
				imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				return imageCreateInfo;
			}

			inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
				VkDescriptorType type,
				VkShaderStageFlags stageFlags,
				uint32_t binding,
				uint32_t descriptorCount = 1)
			{
				VkDescriptorSetLayoutBinding setLayoutBinding{};
				setLayoutBinding.descriptorType = type;// type of descriptor ( uniform, dynamic uniform, image sampler, etc)
				setLayoutBinding.stageFlags = stageFlags; // Shader stage to bind to
				setLayoutBinding.binding = binding;// Binding point in shader (designated by binding number in shader)
				setLayoutBinding.descriptorCount = descriptorCount;// Number of descriptors for binding	

				setLayoutBinding.pImmutableSamplers = nullptr;							// For texture : can make sampler immutable by specifiying in layout
				return setLayoutBinding;
			}

			inline VkImageMemoryBarrier imageMemoryBarrier()
			{
				VkImageMemoryBarrier imageMemoryBarrier{};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				return imageMemoryBarrier;
			}

			inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
				const std::vector<VkDescriptorPoolSize>& poolSizes,
				uint32_t maxSets)
			{
				VkDescriptorPoolCreateInfo descriptorPoolInfo{};
				descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());// Amount of pool sizes being passed
				descriptorPoolInfo.pPoolSizes = poolSizes.data();// Pool sizes to create pool with
				descriptorPoolInfo.maxSets = maxSets; // Maximum number of descriptor sets that can be created from pool
				return descriptorPoolInfo;
			}

			inline VkDescriptorPoolSize descriptorPoolSize(
				VkDescriptorType type,
				uint32_t descriptorCount)
			{
				VkDescriptorPoolSize descriptorPoolSize{};
				descriptorPoolSize.type = type;
				descriptorPoolSize.descriptorCount = descriptorCount;
				return descriptorPoolSize;
			}

			inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
				const VkDescriptorSetLayoutBinding* pBindings,
				uint32_t bindingCount)
			{
				VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
				descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorSetLayoutCreateInfo.pBindings = pBindings;// array of binding infos
				descriptorSetLayoutCreateInfo.bindingCount = bindingCount;// number of binding infos
				return descriptorSetLayoutCreateInfo;
			}

			inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo()
			{
				VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
				pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				return pipelineVertexInputStateCreateInfo;
			}

			inline VkWriteDescriptorSet writeDescriptorSet(
				VkDescriptorSet dstSet,
				VkDescriptorType type,
				uint32_t binding,
				VkDescriptorImageInfo *imageInfo,
				uint32_t descriptorCount = 1)
			{
				VkWriteDescriptorSet writeDescriptorSet {};
				writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSet.dstSet = dstSet;
				writeDescriptorSet.descriptorType = type;
				writeDescriptorSet.dstBinding = binding;
				writeDescriptorSet.pImageInfo = imageInfo;
				writeDescriptorSet.descriptorCount = descriptorCount;
				return writeDescriptorSet;
			}

			inline VkWriteDescriptorSet writeDescriptorSet(
				VkDescriptorSet dstSet,
				VkDescriptorType type,
				uint32_t binding,
				const VkDescriptorBufferInfo* bufferInfo,
				uint32_t descriptorCount = 1)
			{
				VkWriteDescriptorSet writeDescriptorSet {};
				writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSet.dstSet = dstSet;
				writeDescriptorSet.descriptorType = type;
				writeDescriptorSet.dstBinding = binding;
				writeDescriptorSet.pBufferInfo = bufferInfo;
				writeDescriptorSet.descriptorCount = descriptorCount;
				return writeDescriptorSet;
			}

			inline VkVertexInputBindingDescription vertexInputBindingDescription(
				uint32_t binding,
				uint32_t stride,
				VkVertexInputRate inputRate)
			{
				VkVertexInputBindingDescription vInputBindDescription{};
				vInputBindDescription.binding = binding;//can bind multiple streams of data, this defines which one
				vInputBindDescription.stride = stride; //size of a single vertex object in memory
				vInputBindDescription.inputRate = inputRate;//how to move between data after each vertex
															//VK_VERTEX_INPUT_RATE_INSTANCE : move to a vertex for the next instance
				return vInputBindDescription;
			}

			inline VkVertexInputAttributeDescription vertexInputAttributeDescription(
				uint32_t binding,
				uint32_t location,
				VkFormat format,
				uint32_t offset)
			{
				VkVertexInputAttributeDescription vInputAttribDescription{};
				vInputAttribDescription.location = location;//location in shader where data will be read from
				vInputAttribDescription.binding = binding; //which binding the data is at  ( should be same as above)
				vInputAttribDescription.format = format; // format the data will take (also helps define the size of the data) 
				vInputAttribDescription.offset = offset; // where this attribute is defined in the data for a single vertex
				return vInputAttribDescription;
			}


			inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
				const std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions,
				const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions
			)
			{
				VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
				pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
				pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexBindingDescriptions.data(); //list of vertext binding descriptions (data spacing / stride info)
				pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
				pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data(); // list of vertext attribute descriptions (data format and wheret to bind to/from)
				return pipelineVertexInputStateCreateInfo;
			}

			

			inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
				VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				VkPipelineInputAssemblyStateCreateFlags flags = 0,
				VkBool32 primitiveRestartEnable = VK_FALSE) // Typically unused
			{
				VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
				pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				pipelineInputAssemblyStateCreateInfo.topology = topology;
				pipelineInputAssemblyStateCreateInfo.flags = flags;
				pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
				return pipelineInputAssemblyStateCreateInfo;
			}

			inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
				VkPolygonMode polygonMode,
				VkCullModeFlags cullMode,
				VkFrontFace frontFace,
				VkPipelineRasterizationStateCreateFlags flags = 0)
			{
				VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
				pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;			// how to handle filling points between vertices								
				pipelineRasterizationStateCreateInfo.cullMode = cullMode;				// which face of the triangle to cull (backface cull)								
				pipelineRasterizationStateCreateInfo.frontFace = frontFace;				// Winding to determine which side is front								
				pipelineRasterizationStateCreateInfo.flags = flags;
				pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;		// change if fragments beyond/near the far planes are clipped(default) or clamped.. must enable depth clamp in device features								
				pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;					// how thick lines should be when drawn	

				pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;			// Wheter to add debt biast to fragments. (good for stoppging "shadow acne" in shadow mapping)
				pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;	// wheter to desicard data and skip rasterizer. never creates fragments, only suitable for pipeline without framebuffer output.
				return pipelineRasterizationStateCreateInfo;
			}

			inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
				VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				VkPipelineMultisampleStateCreateFlags flags = 0)
			{
				VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
				pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples; //number of samples to use per fragment;
				pipelineMultisampleStateCreateInfo.flags = flags;
				return pipelineMultisampleStateCreateInfo;
			}

			inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
				VkColorComponentFlags colorWriteMask,
				VkBool32 blendEnable)
			{
				VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
				pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask; //colors to apply blending to
				pipelineColorBlendAttachmentState.blendEnable = blendEnable;
				return pipelineColorBlendAttachmentState;
			}

			inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
				uint32_t attachmentCount,
				const VkPipelineColorBlendAttachmentState* pAttachments)
			{
				VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
				pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
				pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
				pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // alternative to calculations is to use logical operations
				return pipelineColorBlendStateCreateInfo;
			}

			inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
				const std::vector<VkDynamicState>& pDynamicStates,
				VkPipelineDynamicStateCreateFlags flags = 0)
			{
				VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
				//dynamic viewport : can resize in command buffer with vkCmdSetViewport(commandbuffer,0,1,&viewport);
				//dynamic viewport : can resize in command buffer with vkCmdSetScissor(commandbuffer,0,1,&scissor);
				pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates.data();
				pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(pDynamicStates.size());
				pipelineDynamicStateCreateInfo.flags = flags;
				return pipelineDynamicStateCreateInfo;
			}

			inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
				VkBool32 depthTestEnable,
				VkBool32 depthWriteEnable,
				VkCompareOp depthCompareOp)
			{
				VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
				pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable; //enable checking depth to determine fragment write
				pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;// enable writing to depth buffer (to replace old values)
				pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;// comparison operation that allows an overwrite (is in front)
				pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
				pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;		// Depth bounds test: does the depth value exist between 2 bounds
				pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;			// Enable stencil test
				return pipelineDepthStencilStateCreateInfo;
			}

			inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
				uint32_t viewportCount = 1,
				uint32_t scissorCount = 1,
				VkPipelineViewportStateCreateFlags flags = 0)
			{
				VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
				pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				pipelineViewportStateCreateInfo.viewportCount = viewportCount;
				pipelineViewportStateCreateInfo.scissorCount = scissorCount;
				pipelineViewportStateCreateInfo.flags = flags;
				return pipelineViewportStateCreateInfo;
			}

			inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
				const VkDescriptorSetLayout* pSetLayouts,
				uint32_t setLayoutCount = 1)
			{
				VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
				pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
				pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
				return pipelineLayoutCreateInfo;
			}

			template<typename T>
            inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const T& setLayouts)
            {
                return pipelineLayoutCreateInfo(setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));
            }

			inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
				VkDescriptorPool descriptorPool,
				const VkDescriptorSetLayout* pSetLayouts,
				uint32_t descriptorSetCount)
			{
				VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
				descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAllocateInfo.descriptorPool = descriptorPool;
				descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
				descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
				return descriptorSetAllocateInfo;
			}

			inline VkGraphicsPipelineCreateInfo pipelineCreateInfo(
				VkPipelineLayout layout,
				VkRenderPass renderPass,
				VkPipelineCreateFlags flags = 0)
			{
				VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
				pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineCreateInfo.layout = layout;         //pipeline layout pipeline should use
				pipelineCreateInfo.renderPass = renderPass;	//render pass description the pipeline is compatible with
				pipelineCreateInfo.flags = flags;

				pipelineCreateInfo.subpass = 0;	//subpass of rneder pass to use with pipeline
				// pipeline derivatives : can create multiple pipels that derive from one another for optimization
				pipelineCreateInfo.basePipelineIndex = -1; //or index of pipeline being created to derive from (in case creating multiple at once)
				pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;  //existing pipeline to derive from
				return pipelineCreateInfo;
			}


			inline VkCommandBufferBeginInfo commandBufferBeginInfo()
			{
				VkCommandBufferBeginInfo cmdBufferBeginInfo{};
				cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				return cmdBufferBeginInfo;
			}

			inline VkRenderPassBeginInfo renderPassBeginInfo()
			{
				VkRenderPassBeginInfo renderPassBeginInfo{};
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				return renderPassBeginInfo;
			}

			// Acts as a rerouter for cleaner code
			template<typename T, typename ... ARGS>
			constexpr T Creator(ARGS&& ... args) // Intentionally spelling this way to avoid use of the common "Create" word
			{
			#define ENTRY(vkType, function) else if constexpr (std::is_same_v<T, vkType>) { return function(std::forward<ARGS>(args)...); }
				if constexpr (std::is_same_v<T, int>) { static_assert(false, "Vulkan API struct creator not implemented!"); }

				ENTRY(VkPipelineInputAssemblyStateCreateInfo, pipelineInputAssemblyStateCreateInfo)
				ENTRY(VkPipelineViewportStateCreateInfo,      pipelineViewportStateCreateInfo)
				ENTRY(VkPipelineVertexInputStateCreateInfo,   pipelineVertexInputStateCreateInfo)
				ENTRY(VkPipelineMultisampleStateCreateInfo,   pipelineMultisampleStateCreateInfo)
				ENTRY(VkPipelineRasterizationStateCreateInfo, pipelineRasterizationStateCreateInfo)
				ENTRY(VkPipelineDynamicStateCreateInfo,	      pipelineDynamicStateCreateInfo)
				ENTRY(VkPipelineDepthStencilStateCreateInfo,  pipelineDepthStencilStateCreateInfo)

			#undef ENTRY
				else { static_assert(false, "Vulkan API struct creator not implemented!"); }
			}

			// Ghetto, could be cleaner... Or just use VK HPP...
			struct PSOCreatorWrapper
			{
				VkPipelineShaderStageCreateInfo pStages;
				VkPipelineVertexInputStateCreateInfo pVertexInputState;
				VkPipelineInputAssemblyStateCreateInfo pInputAssemblyState;
				VkPipelineTessellationStateCreateInfo pTessellationState;
				VkPipelineViewportStateCreateInfo pViewportState;
				VkPipelineRasterizationStateCreateInfo pRasterizationState;
				VkPipelineMultisampleStateCreateInfo pMultisampleState;
				VkPipelineDepthStencilStateCreateInfo pDepthStencilState;
				VkPipelineColorBlendStateCreateInfo pColorBlendState;
				VkPipelineDynamicStateCreateInfo pDynamicState;

				template<typename T, typename ... ARGS>
				void Set(ARGS&& ... args)
				{
#define ENTRY(vkType, var) else if constexpr (std::is_same_v<T, vkType>) { var = Creator<vkType>(std::forward<ARGS>(args)...); }
					if constexpr (std::is_same_v<T, int>) { static_assert(false, "Vulkan API struct creator not implemented!"); }

					ENTRY(VkPipelineShaderStageCreateInfo, pStages)
					ENTRY(VkPipelineVertexInputStateCreateInfo, pVertexInputState)
					ENTRY(VkPipelineInputAssemblyStateCreateInfo, pInputAssemblyState)
					ENTRY(VkPipelineTessellationStateCreateInfo, pTessellationState)
					ENTRY(VkPipelineViewportStateCreateInfo, pViewportState)
					ENTRY(VkPipelineRasterizationStateCreateInfo, pRasterizationState)
					ENTRY(VkPipelineMultisampleStateCreateInfo, pMultisampleState)
					ENTRY(VkPipelineDepthStencilStateCreateInfo, pDepthStencilState)
					ENTRY(VkPipelineColorBlendStateCreateInfo, pColorBlendState)
					ENTRY(VkPipelineDynamicStateCreateInfo, pDynamicState)
					else static_assert(false, "PSOCreatorWrapper Get CreateInfo type invalid!");
#undef ENTRY
				}

				template<typename T>
				constexpr T& Get()
				{
					if constexpr (std::is_same_v<T, VkPipelineShaderStageCreateInfo>) return pStages;
					else if constexpr (std::is_same_v<T, VkPipelineVertexInputStateCreateInfo>) return pVertexInputState;
					else if constexpr (std::is_same_v<T, VkPipelineInputAssemblyStateCreateInfo>) return pInputAssemblyState;
					else if constexpr (std::is_same_v<T, VkPipelineTessellationStateCreateInfo>) return pTessellationState;
					else if constexpr (std::is_same_v<T, VkPipelineViewportStateCreateInfo>) return pViewportState;
					else if constexpr (std::is_same_v<T, VkPipelineRasterizationStateCreateInfo>) return pRasterizationState;
					else if constexpr (std::is_same_v<T, VkPipelineMultisampleStateCreateInfo>) return pMultisampleState;
					else if constexpr (std::is_same_v<T, VkPipelineDepthStencilStateCreateInfo>) return pDepthStencilState;
					else if constexpr (std::is_same_v<T, VkPipelineColorBlendStateCreateInfo>) return pColorBlendState;
					else if constexpr (std::is_same_v<T, VkPipelineDynamicStateCreateInfo>) return pDynamicState;
					else static_assert(false, "PSOCreatorWrapper Get CreateInfo type invalid!");
				}

				constexpr PSOCreatorWrapper& SetRenderPass(VkRenderPass renderPass) 
				{
					m_Data.renderPass = renderPass;
					return *this;
				}

				void SetAndCompile()
				{
					// ???
					m_Data.pStages = &pStages;
					m_Data.pVertexInputState = &pVertexInputState;
					m_Data.pInputAssemblyState = &pInputAssemblyState;
					m_Data.pTessellationState = &pTessellationState;
					m_Data.pViewportState = &pViewportState;
					m_Data.pRasterizationState = &pRasterizationState;
					m_Data.pMultisampleState = &pMultisampleState;
					m_Data.pDepthStencilState = &pDepthStencilState;
					m_Data.pColorBlendState = &pColorBlendState;
					m_Data.pDynamicState = &pDynamicState;
				}

				// The actual one to compile only holds pointers
				VkGraphicsPipelineCreateInfo m_Data;
			};

		}
	}

}

