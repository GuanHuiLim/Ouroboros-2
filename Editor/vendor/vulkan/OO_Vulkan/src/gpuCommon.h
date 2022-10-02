/************************************************************************************//*!
\file           gpuCommon.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines common structures for the renderer

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "vulkan/vulkan.h"
#include "MeshModel.h"
#include <functional>

namespace oGFX
{

struct SetupInfo
{
	bool debug = false;
	bool renderDoc = false;
	bool useOwnImgui = false;
	std::function<void()> SurfaceFunctionPointer{ nullptr };
	std::vector<const char*> extensions;
};

using IndirectCommand = VkDrawIndexedIndirectCommand;

void IndirectCommandsHelper(Node* node, std::vector<oGFX::IndirectCommand>& m_DrawIndirectCommandsCPU, uint32_t& counter);

}
