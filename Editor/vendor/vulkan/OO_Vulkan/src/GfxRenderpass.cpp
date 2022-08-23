#include "GfxRenderpass.h"

RenderPassDatabase* RenderPassDatabase::Get()
{
	if (ms_renderpass == nullptr)
	{
		assert(true); // this shouldnt happen
		ms_renderpass = std::make_unique<RenderPassDatabase>();
	}
	return ms_renderpass.get();
}

void RenderPassDatabase::RegisterRenderPass(std::unique_ptr<GfxRenderpass>&& renderPass)
{
	renderPass->m_Index = m_RegisteredRenderPasses++;
	m_AllRenderPasses.emplace_back(std::move(renderPass));
}

void RenderPassDatabase::InitAllRegisteredPasses()
{
	auto renderpasses = Get();
	
	for (auto& renderPass : renderpasses->m_AllRenderPasses)
	{
		renderPass->Init();
	}

    for (auto& renderPass : renderpasses->m_AllRenderPasses)
    {
        renderPass->CreatePSO();
    }
}

void RenderPassDatabase::ShutdownAllRegisteredPasses()
{
    auto renderpasses = Get();

    for (auto& renderPass : renderpasses->m_AllRenderPasses)
    {
        renderPass->Shutdown();
    }
}