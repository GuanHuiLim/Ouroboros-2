/************************************************************************************//*!
\file           Audio.cpp
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Sep 22, 2022
\brief          Contains the definition for the Audio framework.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

#include "Audio.h"

#include <array>
#include <list>
#include <limits>

#include <fmod_errors.h>

#include "Ouroboros/ECS/ECS.h"

namespace oo
{
    namespace audio
    {
        constexpr SoundID MAX_SOUNDS = std::numeric_limits<SoundID>::max();
        constexpr SoundID INVALID_SOUND_ID = (SoundID)-1;

        FMOD::System* system;
        FMOD::ChannelGroup* channelGroupGlobal;
        std::array<FMOD::Sound*, MAX_SOUNDS> sounds;
        SoundID soundIDNext;

        [[nodiscard]] static SoundID GetSoundID()
        {
            // Get the next sound ID available
            SoundID next = soundIDNext;

            // Check if next sound ID exceeds size
            if (next >= MAX_SOUNDS)
                return INVALID_SOUND_ID;

            // Proceed sequentially to next nullptr
            do
                ++soundIDNext;
            while (soundIDNext < MAX_SOUNDS && sounds[soundIDNext] != nullptr);

            return next;
        }

        static void ReleaseSoundID(const SoundID& id)
        {
            if (id < soundIDNext)
                soundIDNext = id;
        }

        void Init(size_t channelCount)
        {
            // Create system
            FMOD_ERR_HAND(FMOD::System_Create(&system));

            // Initialise system
            FMOD_ERR_HAND(system->init(static_cast<int>(channelCount), FMOD_INIT_NORMAL, 0));

            // Create channel groups
            FMOD_ERR_HAND(system->createChannelGroup("Global", &channelGroupGlobal));

            // Initialise sounds
            std::fill(sounds.begin(), sounds.end(), nullptr);
            soundIDNext = 0;
        }

        void Update()
        {
            // Update system
            FMOD_ERR_HAND(system->update());

            // Handle components
        }

        void ShutDown()
        {
            // Release sounds
            for (int i = 0; i < sounds.size(); ++i)
            {
                if (!sounds[i])
                    continue;
                FMOD_ERR_HAND(sounds[i]->release());
                sounds[i] = nullptr;
            }

            // Release channel groups
            FMOD_ERR_HAND(channelGroupGlobal->release());

            // Release system
            FMOD_ERR_HAND(system->release());
        }

        FMOD::System* GetSystem()
        {
            return system;
        }

        FMOD::Sound* GetSound(const SoundID& id)
        {
            if (id >= 0 && id < MAX_SOUNDS)
            {
                return sounds[id];
            }
            return nullptr;
        }

        SoundID CreateSound(const std::filesystem::path& path)
        {
            SoundID id = GetSoundID();
            if (id != INVALID_SOUND_ID)
            {
                FMOD::Sound* sound;
                FMOD_ERR_HAND(audio::GetSystem()->createSound(path.string().c_str(), FMOD_DEFAULT, nullptr, &sound));
                sounds[id] = sound;
            }
            return id;
        }

        void FreeSound(const SoundID& id)
        {
            if (id >= 0 && id < MAX_SOUNDS)
            {
                if (!sounds[id])
                    return;
                FMOD_ERR_HAND(sounds[id]->release());
                sounds[id] = nullptr;
                ReleaseSoundID(id);
            }
        }

        FMOD::Channel* PlayGlobalOneShot(const SoundID& id)
        {
            if (!sounds.at(id))
                return nullptr;

            FMOD::Sound* sound = sounds.at(id);
            sound->setMode(FMOD_LOOP_OFF);
            FMOD::Channel* channel;
            FMOD_ERR_HAND(system->playSound(sound, channelGroupGlobal, false, &channel));
            channel->setLoopCount(0);
            return channel;
        }

        FMOD::Channel* PlayGlobalLooping(const SoundID& id, int loopCount)
        {
            if (!sounds.at(id))
                return nullptr;

            FMOD::Sound* sound = sounds.at(id);
            sound->setMode(FMOD_LOOP_NORMAL);
            FMOD::Channel* channel;
            FMOD_ERR_HAND(system->playSound(sound, channelGroupGlobal, false, &channel));
            channel->setLoopCount(loopCount);
            return channel;
        }

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
