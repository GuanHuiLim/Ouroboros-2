/************************************************************************************//*!
\file           AudioSystem.cpp
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Sep 22, 2022
\brief          Contains the definition for the Audio System.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

#include "AudioSystem.h"

#include <fmod_errors.h>

namespace oo
{
    AudioSystem::AudioSystem(size_t channelCount)
    {
        // Create system
        FMOD_ERROR_HANDLER(FMOD::System_Create(&system));

        // Initialise system
        FMOD_ERROR_HANDLER(system->init(channelCount, FMOD_INIT_NORMAL, 0));

        // Create channel groups
        FMOD_ERROR_HANDLER(system->createChannelGroup("One-Shots", &channelGroupOneShots));
    }

    AudioSystem::~AudioSystem()
    {
        // Release channel groups
        channelGroupOneShots->release();

        // Release system
        system->release();
    }

    // TEMPORARY IMPLEMENTATION TO SHOWCASE FUNCTIONAL AUDIO PLAYBACK
    void AudioSystem::PlayOneShot(const std::filesystem::path& path)
    {
        FMOD::Sound* sound;
        FMOD_ERROR_HANDLER(system->createSound(path.string().c_str(), FMOD_DEFAULT, nullptr, &sound));
        FMOD::Channel* channel;
        FMOD_ERROR_HANDLER(system->playSound(sound, channelGroupOneShots, false, &channel));
    }

    bool AudioSystem::ErrorHandler(FMOD_RESULT result, const char* file, int line)
    {
        if (result != FMOD_OK)
        {
            std::stringstream err;
            err << file << "(" << line << "): " << FMOD_ErrorString(result) << std::endl;
            std::cout << err.str();
            return true;
        }
        return false;
    }
}
