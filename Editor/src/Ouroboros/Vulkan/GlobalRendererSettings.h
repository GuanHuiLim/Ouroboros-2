/************************************************************************************//*!
\file           GlobalRendererSettings.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Sept 24, 2022
\brief          Defines data relevant to the renderer at a global level.
                Data here will be serialized and can be tweak via the editor to produce
                differing results in the scene.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

namespace oo
{
    struct GlobalRendererSettings
    {
        struct SSAOSettings
        {
            float Radius = 0.5f;
            float Bias = 0.025f;
            RTTR_ENABLE();
        }
        SSAO{};

        struct LightingSettings
        {
            float Ambient = 0.2f;
            float MaxBias = 0.0001f;
            float BiasMultiplier = 0.002f;

            void SetMaxBias(float);
            float GetMaxBias() const;

            void SetAmbient(float);
            float GetAmbient() const;

            RTTR_ENABLE();
        }
        Lighting{};

        RTTR_ENABLE();
    };
	class RendererSettings
	{
	public:
		inline static GlobalRendererSettings setting;
	};
}
