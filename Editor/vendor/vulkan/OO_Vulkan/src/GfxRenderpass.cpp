#include "GfxRenderpass.h"

RenderPassDatabase* RenderPassDatabase::ms_renderpass{ nullptr };

RenderPassDatabase* RenderPassDatabase::Get()
{
	if (ms_renderpass == nullptr)
	{
		//assert(true); // this shouldnt happen
		//ms_renderpass = std::make_unique<RenderPassDatabase>();
		ms_renderpass =new RenderPassDatabase;
	}
	//std::cout << ms_renderpass.get() << std::endl;
	//return ms_renderpass.get();
	return ms_renderpass;
}


void RenderPassDatabase::RegisterRenderPass(GfxRenderpass* renderPass)
{
	renderPass->m_Index = m_RegisteredRenderPasses++;
	m_AllRawRenderPasses.push_back(renderPass);
}

void RenderPassDatabase::InitAllRegisteredPasses()
{
	auto renderpasses = Get();
	
	for (auto& renderPass : renderpasses->m_AllRawRenderPasses)
	{
		renderPass->Init();
	}

    for (auto& renderPass : renderpasses->m_AllRawRenderPasses)
    {
        renderPass->CreatePSO();
    }
}

void RenderPassDatabase::ShutdownAllRegisteredPasses()
{
    auto renderpasses = Get();

    for (auto& renderPass : renderpasses->m_AllRawRenderPasses)
    {
        renderPass->Shutdown();
		delete renderPass;
    }
}