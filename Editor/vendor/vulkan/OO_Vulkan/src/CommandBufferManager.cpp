/************************************************************************************//*!
\file           CommandBufferManager.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Aug 24, 2023
\brief              Defines command list allocator

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "gpuCommon.h"
#include <algorithm>
#include "VulkanDevice.h"
#include "VulkanUtils.h"
#include "CommandBufferManager.h"
#include "Profiling.h"


VkResult oGFX::CommandBufferManager::InitPool(VkDevice device, uint32_t queueIndex)
{
    assert(device);
    m_device = device;

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueIndex;

    VkResult res{};
    res = vkCreateCommandPool(device, &poolInfo, nullptr, &m_commandpool);
    VK_CHK(res);
    VK_NAME(device, "commandPool", m_commandpool);

    return res;
}

VkCommandBuffer oGFX::CommandBufferManager::GetNextCommandBuffer(bool begin)
{
    if (nextIdx == commandBuffers.size()) 
    {
        AllocateCommandBuffer();
    }
    auto idx = nextIdx++;
    VkCommandBuffer result = commandBuffers[idx];
    if (begin) {
        VkCommandBufferBeginInfo cmdBufInfo = oGFX::vkutils::inits::commandBufferBeginInfo();
        vkBeginCommandBuffer(result, &cmdBufInfo);
        submitted[idx] = eRECSTATUS::RECORDING;
    }
    return result;
}

void oGFX::CommandBufferManager::EndCommandBuffer(VkCommandBuffer cmd)
{
    size_t idx = FindCmdIdx(cmd);
    OO_ASSERT(submitted[idx] == eRECSTATUS::RECORDING);
    // end command
    vkEndCommandBuffer(cmd);
    submitted[idx] = eRECSTATUS::ENDED;
}

void oGFX::CommandBufferManager::ResetPool()
{
    nextIdx = 0;

    VkCommandPoolResetFlags flags{ VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT };

    VK_CHK(vkResetCommandPool(m_device, m_commandpool, flags));
    std::fill(submitted.begin(), submitted.end(), eRECSTATUS::INVALID);

    counter = 0;
}

void oGFX::CommandBufferManager::DestroyPool()
{
    if (m_commandpool);
    vkDestroyCommandPool(m_device, m_commandpool, nullptr);
    m_commandpool = VK_NULL_HANDLE;
}

void oGFX::CommandBufferManager::SubmitCommandBuffer(VkQueue queue, VkCommandBuffer cmd)
{
    size_t idx = FindCmdIdx(cmd);

    if (submitted[idx] == eRECSTATUS::RECORDING) 
    {
        // End commands
        vkEndCommandBuffer(cmd);
    }
    submitted[idx] = eRECSTATUS::SUBMITTED;
    

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    PROFILE_SCOPED("SUBMIT SINGLE");
    //assuming we have only a few meshes to load we will pause here until we load the previous object
    //submit transfer commands to transfer queue and wait until it finishes
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
}

void oGFX::CommandBufferManager::SubmitCommandBufferAndWait(VkQueue queue, VkCommandBuffer cmd)
{
    SubmitCommandBuffer(queue, cmd);
    vkQueueWaitIdle(queue);
}

void oGFX::CommandBufferManager::SubmitAll(VkQueue queue, VkSubmitInfo inInfo, VkFence signalFence)
{
    std::vector<VkCommandBuffer> buffers;
    buffers.reserve(commandBuffers.size());
    for (size_t i = 0; i < submitted.size(); i++)
    {
        if (submitted[i] == eRECSTATUS::RECORDING)
        {
            PROFILE_SCOPED("END CMDBUFFER");
            vkEndCommandBuffer(commandBuffers[i]);
            submitted[i] = eRECSTATUS::ENDED;
        }

        if (submitted[i] == eRECSTATUS::ENDED)
        {
            buffers.emplace_back(commandBuffers[i]);           
        }
    }
    //for (auto cmd : buffers)
    //{
    //    
    //}
    for (auto& sub : submitted)
    {
        if (sub == eRECSTATUS::ENDED)
            sub = eRECSTATUS::SUBMITTED;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = buffers.size();
    submitInfo.pCommandBuffers = buffers.data();

    submitInfo.pSignalSemaphores = inInfo.pSignalSemaphores;
    submitInfo.signalSemaphoreCount = inInfo.signalSemaphoreCount;

    submitInfo.pWaitSemaphores= inInfo.pWaitSemaphores;
    submitInfo.waitSemaphoreCount = inInfo.waitSemaphoreCount;
    submitInfo.pWaitDstStageMask = inInfo.pWaitDstStageMask;

    submitInfo.pNext = inInfo.pNext;

    //assuming we have only a few meshes to load we will pause here until we load the previous object
    //submit transfer commands to transfer queue and wait until it finishes
    {
        PROFILE_SCOPED("SUBMIT QUEUE");
        vkQueueSubmit(queue, 1, &submitInfo, signalFence);
    }

}

size_t oGFX::CommandBufferManager::FindCmdIdx(VkCommandBuffer cmd)
{
    auto iter = std::find(commandBuffers.begin(), commandBuffers.end(), cmd);
    OO_ASSERT(iter != commandBuffers.end() && "invalid usage");
    auto idx = iter - commandBuffers.begin();

    return idx;
}

void oGFX::CommandBufferManager::AllocateCommandBuffer()
{
    //std::cout << __FUNCTION__ << std::endl;
	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// VK_COMMAND_BUFFER_LEVEL_PRIMARY : buffer you submit directly to queue, cant be called  by other buffers
	//VK_COMMAND_BUFFER_LEVEL_SECONDARY :  buffer cant be called directly, can be called from other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = 1;
	cbAllocInfo.commandPool = m_commandpool;

	VkCommandBuffer cb;

	VkResult result = vkAllocateCommandBuffers(m_device, &cbAllocInfo, &cb);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Failed to allocate Command Buffers!" << std::endl;
		__debugbreak();
	}

    commandBuffers.emplace_back(cb);
    submitted.emplace_back(eRECSTATUS::INVALID);
}

