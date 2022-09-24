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
    using SoundID = int16_t;

    namespace audio
    {
        /* --------------------------------------------------------------------------- */
        /* System Lifecycle                                                            */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Initialises the audio system.
        /// You must call this function before any other audio functions.
        /// </summary>
        /// <param name="channelCount">The maximum number of channels.</param>
        void Init(size_t channelCount = 255);
        
        /// <summary>
        /// Updates the audio system.
        /// </summary>
        void Update();

        /// <summary>
        /// Shuts down the audio system.
        /// </summary>
        void ShutDown();

        /* --------------------------------------------------------------------------- */
        /* Getters                                                                     */
        /* --------------------------------------------------------------------------- */

        [[nodiscard]] inline FMOD::System* GetSystem();

        /* --------------------------------------------------------------------------- */
        /* Sound Lifecycle                                                             */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Creates a sound given a file path.
        /// </summary>
        /// <param name="path">The file path.</param>
        /// <returns>The ID of the sound.</returns>
        SoundID CreateSound(const std::filesystem::path& path);

        /// <summary>
        /// Releases a sound.
        /// </summary>
        /// <param name="id">The ID of the sound.</param>
        void FreeSound(const SoundID& id);

        /* --------------------------------------------------------------------------- */
        /* Global Audio                                                                */
        /*                                                                             */
        /* The following functions implement a global audio system, where audio        */
        /* is played without concern for position, attenuation, etc.                   */
        /* These functions implement audio playback that exposes more native control.  */
        /* If possible, use Audio Sources (TODO) instead.                              */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Plays a sound once, globally.
        /// </summary>
        /// <param name="id">The ID of the sound.</param>
        /// <returns>The channel the sound is playing on.</returns>
        FMOD::Channel* PlayGlobalOneShot(const SoundID& id);

        /// <summary>
        /// Plays a sound repeatedly, globally.
        /// </summary>
        /// <param name="id">The ID of the sound.</param>
        /// <param name="loopCount">The number of times to repeat, where -1 represents infinity.</param>
        /// <returns>The channel the sound is playing on.</returns>
        FMOD::Channel* PlayGlobalLooping(const SoundID& id, int loopCount = -1);

        /// <summary>
        /// Stops all global sounds.
        /// </summary>
        void StopGlobal();

        // TEMPORARY IMPLEMENTATION TO SHOWCASE FUNCTIONAL AUDIO PLAYBACK
        void PlayGlobalOneShot(const std::filesystem::path& path);
        void PlayGlobalLooping(const std::filesystem::path& path);

        /* --------------------------------------------------------------------------- */
        /* Utility                                                                     */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Handles FMOD function call results.
        /// </summary>
        /// <param name="result">The function call result.</param>
        /// <param name="file">The file.</param>
        /// <param name="line">The line number.</param>
        /// <returns>Whether the function failed.</returns>
        bool ErrorHandler(FMOD_RESULT result, const char* file, int line);
    }
}
