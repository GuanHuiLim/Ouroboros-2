/************************************************************************************//*!
\file           Audio.h
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Sep 22, 2022
\brief          Contains the declaration for the Audio System.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <filesystem>

#include <fmod.hpp>

//#include "RendererComponent.h"
//#include "Archetypes_Ecs/src/A_Ecs.h"
//#include "Ouroboros/Scene/Scene.h"
//#include "Ouroboros/Transform/TransformComponent.h"

#define FMOD_ERR_HAND(result) oo::audio::ErrorHandler(result, __FILE__, __LINE__);

namespace oo
{
    namespace audio
    {
        void Init(size_t channelCount = 255);
        void Update();
        void ShutDown();

        [[nodiscard]] inline FMOD::System* GetSystem();

        // TEMPORARY IMPLEMENTATION TO SHOWCASE FUNCTIONAL AUDIO PLAYBACK
        void PlayGlobalOneShot(const std::filesystem::path& path);
        void PlayGlobalLooping(const std::filesystem::path& path);
        void StopGlobal();

        bool ErrorHandler(FMOD_RESULT result, const char* file, int line);
    }
}
