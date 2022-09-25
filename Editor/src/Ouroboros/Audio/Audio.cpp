/************************************************************************************//*!
\file           Audio.cpp
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

#include "Audio.h"

#include <fmod_errors.h>

namespace oo
{
    namespace audio
    {
        FMOD::System* system;
        FMOD::ChannelGroup* channelGroupGlobal;

        void Init(size_t channelCount)
        {
            // Create system
            FMOD_ERR_HAND(FMOD::System_Create(&system));

            // Initialise system
            FMOD_ERR_HAND(system->init(channelCount, FMOD_INIT_NORMAL, 0));

            // Create channel groups
            FMOD_ERR_HAND(system->createChannelGroup("One-Shots", &channelGroupGlobal));
        }

        void Update()
        {
            // Update system
            FMOD_ERR_HAND(system->update());
        }

        void ShutDown()
        {
            // Release channel groups
            FMOD_ERR_HAND(channelGroupGlobal->release());

            // Release system
            FMOD_ERR_HAND(system->release());
        }

        FMOD::System* GetSystem()
        {
            return system;
        }

        // TEMPORARY IMPLEMENTATION TO SHOWCASE FUNCTIONAL AUDIO PLAYBACK
        void PlayGlobalOneShot(const std::filesystem::path& path)
        {
            FMOD::Sound* sound;
            FMOD_ERR_HAND(system->createSound(path.string().c_str(), FMOD_DEFAULT, nullptr, &sound));
            FMOD::Channel* channel;
            FMOD_ERR_HAND(system->playSound(sound, channelGroupGlobal, false, &channel));
        }

        // TEMPORARY IMPLEMENTATION TO SHOWCASE FUNCTIONAL AUDIO PLAYBACK
        void PlayGlobalLooping(const std::filesystem::path& path)
        {
            FMOD::Sound* sound;
            FMOD_ERR_HAND(system->createSound(path.string().c_str(), FMOD_LOOP_NORMAL, nullptr, &sound));
            FMOD::Channel* channel;
            FMOD_ERR_HAND(system->playSound(sound, channelGroupGlobal, false, &channel));
        }

        // TEMPORARY IMPLEMENTATION TO SHOWCASE FUNCTIONAL AUDIO PLAYBACK
        void StopGlobal()
        {
            FMOD_ERR_HAND(channelGroupGlobal->stop());
        }

        bool ErrorHandler(FMOD_RESULT result, const char* file, int line)
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

}
