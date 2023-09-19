/************************************************************************************//*!
\file           GfxRenderpass.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines base renderpass interface object

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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

void RenderPassDatabase::Shutdown()
{
	if (ms_renderpass)
	{
		ShutdownAllRegisteredPasses();
		delete ms_renderpass;
		ms_renderpass = nullptr;
	}
}


void RenderPassDatabase::RegisterRenderPass(GfxRenderpass* renderPass)
{
	renderPass->m_Index = m_RegisteredRenderPasses++;
	m_AllRawRenderPasses.push_back(renderPass);
}

void RenderPassDatabase::ReloadAllShaders()
{
	// I know... I will be removing this database eventually
	auto renderpasses = Get();
	for (auto& renderPass : renderpasses->m_AllRawRenderPasses)
	{
		renderPass->CreatePSO();
	}

}

void RenderPassDatabase::InitAllRegisteredPasses()
{
	auto renderpasses = Get();
	
	for (auto& renderPass : renderpasses->m_AllRawRenderPasses)
	{
		renderPass->Init();
	}

	ReloadAllShaders();
}

void RenderPassDatabase::ShutdownAllRegisteredPasses()
{
    auto renderpasses = Get();

    for (auto& renderPass : renderpasses->m_AllRawRenderPasses)
    {
        renderPass->Shutdown();
    }
	renderpasses->m_AllRawRenderPasses.clear();
}
