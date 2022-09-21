/************************************************************************************//*!
\file           AudioSystem.h
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

#define FMOD_ERROR_HANDLER(result) oo::AudioSystem::ErrorHandler(result, __FILE__, __LINE__);

namespace oo
{
	class AudioSystem
	{
	public:
		AudioSystem(size_t channelCount);
		~AudioSystem();

		// TEMPORARY IMPLEMENTATION TO SHOWCASE FUNCTIONAL AUDIO PLAYBACK
		void PlayOneShot(const std::filesystem::path& path);

		static bool ErrorHandler(FMOD_RESULT result, const char* file, int line);

	private:
		FMOD::System* system;
		FMOD::ChannelGroup* channelGroupOneShots; // channel group for one-shot non-positional audio
	};
}
