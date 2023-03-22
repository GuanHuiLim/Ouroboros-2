#pragma once

#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/Vulkan/MeshRendererComponent.h"
#include "Ouroboros/Vulkan/SkinRendererComponent.h"

namespace oo
{
	SCRIPT_API_GET_SET(MeshRendererComponent, EmissiveColor, Color, EmissiveColor)


	SCRIPT_API_GET_SET(SkinMeshRendererComponent, EmissiveColor, Color, EmissiveColor)
}