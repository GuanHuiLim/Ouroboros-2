#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/Vulkan/GlobalRendererSettings.h"
#include "Ouroboros/EventSystem/EventTypes.h"

namespace oo
{
#define SCRIPT_API_EXPORT_RENDERER_SETTINGS_FUNC(Type, Struct, GetFunc, SetFunc) \
SCRIPT_API Type RendererSettings_##Struct##_##GetFunc() \
{ \
	return RendererSettings::setting.Struct.GetFunc();\
} \
SCRIPT_API void RendererSettings_##Struct##_##SetFunc(Type value) \
{ \
	RendererSettings::setting.Struct.SetFunc(value);\
	UpdateRendererSettings e; \
	EventManager::Broadcast<UpdateRendererSettings>(&e); \
}

#define SCRIPT_API_EXPORT_RENDERER_SETTINGS(Type, Struct, Variable) \
SCRIPT_API Type RendererSettings_##Struct##_Get##Variable() \
{ \
	return RendererSettings::setting.Struct.Variable;\
} \
SCRIPT_API void RendererSettings_##Struct##_Set##Variable(Type value) \
{ \
	RendererSettings::setting.Struct.Variable = value;\
	UpdateRendererSettings e; \
	EventManager::Broadcast<UpdateRendererSettings>(&e); \
}

	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, SSAO, Radius)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, SSAO, Bias)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, SSAO, intensity)

	SCRIPT_API_EXPORT_RENDERER_SETTINGS_FUNC(float, Lighting, GetAmbient, SetAmbient)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS_FUNC(float, Lighting, GetMaxBias, SetMaxBias)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, Lighting, BiasMultiplier)

	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, Bloom, Threshold)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, Bloom, SoftThreshold)

	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, ColourCorrection, HighlightThreshold)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, ColourCorrection, ShadowThreshold)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(Color, ColourCorrection, ShadowColour)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(Color, ColourCorrection, MidtonesColour)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(Color, ColourCorrection, HighlightColour)

	SCRIPT_API_EXPORT_RENDERER_SETTINGS(Color, Vignette, Colour)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, Vignette, InnerRadius)
	SCRIPT_API_EXPORT_RENDERER_SETTINGS(float, Vignette, OuterRadius)
}