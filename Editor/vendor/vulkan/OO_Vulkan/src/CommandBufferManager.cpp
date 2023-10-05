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
#include <thread>
#include "VulkanDevice.h"
#include "VulkanUtils.h"
#include "CommandBufferManager.h"
#include "Profiling.h"

const size_t MAX_THREADS = std::thread::hardware_concurrency();

VkResult oGFX::CommandBufferManager::InitPool(VkDevice device, uint32_t queueIndex)
{
    assert(device);
    m_device = device;

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueIndex;

    nextIndices.resize(MAX_THREADS);
    threadSubmitteds.resize(MAX_THREADS);
    threadCBs.resize(MAX_THREADS);
    m_commandpools.resize(MAX_THREADS);
    for (size_t i = 0; i < MAX_THREADS; i++)
    {
        VK_CHK(vkCreateCommandPool(device, &poolInfo, nullptr, &m_commandpools[i]));
        VK_NAME(device, ("commandPool_t" + std::to_string(i)).c_str(), m_commandpools[i]);
    }  

    orderedCommands.reserve(100);

    return VK_SUCCESS;
}

VkCommandBuffer oGFX::CommandBufferManager::GetNextCommandBuffer(uint32_t thread_id, bool begin)
{
    auto& submitted = threadSubmitteds[thread_id];
    auto& nextIndex = nextIndices[thread_id];
    auto& commandBuffers = threadCBs[thread_id];

    if (nextIndex == commandBuffers.size())
    {
        AllocateCommandBuffer(thread_id);
    }
    auto bufferIdx = nextIndex++;
    VkCommandBuffer result = commandBuffers[bufferIdx];
    if (begin) {
        VkCommandBufferBeginInfo cmdBufInfo = oGFX::vkutils::inits::commandBufferBeginInfo();
        vkBeginCommandBuffer(result, &cmdBufInfo);
        submitted[bufferIdx] = eRECSTATUS::RECORDING;
    }  
    //printf("Thread [%llu][%2u] order, got cmd [%x]\n", std::this_thread::get_id(),thread_id, result);
    return result;
}

void oGFX::CommandBufferManager::EndCommandBuffer(uint32_t thread_id, VkCommandBuffer cmd)
{
    auto& submitted = threadSubmitteds[thread_id];

    size_t idx = FindCmdIdx(thread_id, cmd);
    OO_ASSERT(submitted[idx] == eRECSTATUS::RECORDING);
    // end command
    vkEndCommandBuffer(cmd);
    submitted[idx] = eRECSTATUS::ENDED;
}

void oGFX::CommandBufferManager::ResetPools()
{
    for (size_t i = 0; i < MAX_THREADS; i++)
    {
        nextIndices[i] = 0;
        VkCommandPoolResetFlags flags{ VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT };
        VK_CHK(vkResetCommandPool(m_device, m_commandpools[i], flags));
        std::fill(threadSubmitteds[i].begin(), threadSubmitteds[i].end(), eRECSTATUS::INVALID);
    }

    // starting a new frame should clear all submissions
    orderedCommands.clear();
}

void oGFX::CommandBufferManager::DestroyPools()
{
    for (size_t i = 0; i < MAX_THREADS; i++)
    {
        if (m_commandpools[i]);
        vkDestroyCommandPool(m_device, m_commandpools[i], nullptr);
        m_commandpools[i] = VK_NULL_HANDLE;
    }    
}

void oGFX::CommandBufferManager::QueueCommandBuffer(VkCommandBuffer cmd)
{
    size_t idx{ size_t(-1) };
    uint32_t thread_id{ uint32_t(-1) };
    //printf("Queueing cmd[%x]\n", cmd);
    FindCmdBufferPool(cmd, thread_id, idx);
    auto& submitted = threadSubmitteds[thread_id];
    if (submitted[idx] == eRECSTATUS::RECORDING)
    {
        // End commands
        vkEndCommandBuffer(cmd);
    }
    submitted[idx] = eRECSTATUS::SUBMITTED;

    orderedCommands.emplace_back(cmd);
}

void oGFX::CommandBufferManager::QueueCommandBuffers(std::vector<VkCommandBuffer>& cmds)
{
    orderedCommands.reserve(orderedCommands.size() + cmds.size());
    for (size_t i = 0; i < cmds.size(); i++)
    {
        QueueCommandBuffer(cmds[i]);
    }
}

void oGFX::CommandBufferManager::SubmitCommandBuffer(VkQueue queue, VkCommandBuffer cmd)
{
    size_t idx{ size_t(-1) };
    uint32_t thread_id{ uint32_t(-1) };
    FindCmdBufferPool(cmd, thread_id, idx);    

    auto& submitted = threadSubmitteds[thread_id];
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
    // batch together entire submit
    // we place un-ordered commands first as we assume they precede all
    std::vector<VkCommandBuffer> submitBatch;
    submitBatch.reserve(50);
    for (size_t thread_id = 0; thread_id < MAX_THREADS; thread_id++)
    {
        auto& commandBuffers = threadCBs[thread_id];
        auto& submitted = threadSubmitteds[thread_id];
        submitBatch.reserve(submitBatch.size() + commandBuffers.size());
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
                submitBatch.emplace_back(commandBuffers[i]);
            }
        }
        for (auto& sub : submitted)
        {
            if (sub == eRECSTATUS::ENDED)
                sub = eRECSTATUS::SUBMITTED;
        }
    }

    // now we append the ordered commands at the back of the unordered stuff
    size_t lastSz = submitBatch.size();
    submitBatch.resize(orderedCommands.size() + submitBatch.size());
    for (size_t i = 0; i < orderedCommands.size(); i++)
    {
        submitBatch[lastSz + i] = orderedCommands[i];
    }
    orderedCommands.clear();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = submitBatch.size();
    submitInfo.pCommandBuffers = submitBatch.data();

    submitInfo.pSignalSemaphores = inInfo.pSignalSemaphores;
    submitInfo.signalSemaphoreCount = inInfo.signalSemaphoreCount;

    submitInfo.pWaitSemaphores= inInfo.pWaitSemaphores;
    submitInfo.waitSemaphoreCount = inInfo.waitSemaphoreCount;
    submitInfo.pWaitDstStageMask = inInfo.pWaitDstStageMask;

    submitInfo.pNext = inInfo.pNext;
    {
        PROFILE_SCOPED("SUBMIT QUEUE");
        vkQueueSubmit(queue, 1, &submitInfo, signalFence);        
    }
}

size_t oGFX::CommandBufferManager::FindCmdIdx(uint32_t thread_id, VkCommandBuffer cmd)
{    
    auto& commandBuffers = threadCBs[thread_id];

    auto iter = std::find(commandBuffers.begin(), commandBuffers.end(), cmd);
    if (iter == commandBuffers.end()) {
        return size_t(-1);
    }
    auto idx = iter - commandBuffers.begin();
    return idx;    
}

void oGFX::CommandBufferManager::FindCmdBufferPool(VkCommandBuffer cmd, uint32_t& outThread, size_t& outIndex)
{
    for (size_t i = 0; i < MAX_THREADS; i++)
    {
        outIndex = FindCmdIdx(i, cmd);
        if (outIndex != size_t(-1))
        {
            outThread = i;
            break;
        }
    }
    OO_ASSERT(outIndex != size_t(-1) && "Invalid usage, commandbuffer doesnt exist");
}

void oGFX::CommandBufferManager::AllocateCommandBuffer(uint32_t thread_id)
{
    //std::cout << __FUNCTION__ << std::endl;
	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// VK_COMMAND_BUFFER_LEVEL_PRIMARY : buffer you submit directly to queue, cant be called  by other buffers
	//VK_COMMAND_BUFFER_LEVEL_SECONDARY :  buffer cant be called directly, can be called from other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = 1;
	cbAllocInfo.commandPool = m_commandpools[thread_id];

	VkCommandBuffer cb;
	VkResult result = vkAllocateCommandBuffers(m_device, &cbAllocInfo, &cb);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Failed to allocate Command Buffers!" << std::endl;
		__debugbreak();
	}

    threadCBs[thread_id].emplace_back(cb);
    threadSubmitteds[thread_id].emplace_back(eRECSTATUS::INVALID);
}

