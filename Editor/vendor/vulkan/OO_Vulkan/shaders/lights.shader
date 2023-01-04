#include "shared_structs.h"

layout(std430, set = 0, binding = 7) readonly buffer Lights
{
	LocalLightInstance Lights_SSBO[];
};
