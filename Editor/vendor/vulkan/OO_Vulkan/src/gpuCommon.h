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