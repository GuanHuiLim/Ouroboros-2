/************************************************************************************//*!
\file           AudioSourceComponent.h
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Sep 25, 2022
\brief          Contains the declaration for the Audio Source component.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <fmod.hpp>
#include <rttr/type>

#include "Ouroboros/Asset/AssetManager.h"

namespace oo
{
    class AudioSourceComponent
    {
    public:
        /* --------------------------------------------------------------------------- */
        /* Type Definitions                                                            */
        /* --------------------------------------------------------------------------- */


        /* --------------------------------------------------------------------------- */
        /* Getters                                                                     */
        /* --------------------------------------------------------------------------- */

        [[nodiscard]] inline Asset GetAudioClip() const { return audioClip; }
        [[nodiscard]] inline FMOD::Channel* GetChannel() const { return channel; }
        [[nodiscard]] inline bool IsPlayOnAwake() const { return playOnAwake; }
        [[nodiscard]] inline bool IsMuted() const { return muted; }
        [[nodiscard]] inline bool IsLoop() const { return loop; }
        [[nodiscard]] inline unsigned int GetLoopBegin() const { return loopBegin; }
        [[nodiscard]] inline unsigned int GetLoopEnd() const { return loopEnd; }
        [[nodiscard]] inline float GetVolume() const { return volume; }
        [[nodiscard]] inline float GetPitch() const { return pitch; }
        [[nodiscard]] bool IsPlaying() const;
        [[nodiscard]] float GetPlaybackTime() const;
        [[nodiscard]] unsigned int GetPlaybackTimeSamples() const;

        /* --------------------------------------------------------------------------- */
        /* Setters                                                                     */
        /* --------------------------------------------------------------------------- */

        void SetAudioClip(Asset a);
        void SetMuted(bool m);
        void SetPlayOnAwake(bool p);
        void SetLoop(bool l);
        void SetLoopBegin(unsigned int t);
        void SetLoopEnd(unsigned int t);
        void SetVolume(float v);
        void SetPitch(float p);
        void SetPlaybackTime(float t);
        void SetPlaybackTimeSamples(unsigned int t);

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Plays the Audio Source.
        /// </summary>
        void Play();

        /// <summary>
        /// Stops the Audio Source.
        /// </summary>
        void Stop();

        /// <summary>
        /// Pauses the Audio Source.
        /// </summary>
        void Pause();

        /// <summary>
        /// Resumes the Audio Source.
        /// </summary>
        void UnPause();

        RTTR_ENABLE();

    private:
        /* --------------------------------------------------------------------------- */
        /* Serialised Members                                                          */
        /* --------------------------------------------------------------------------- */

        Asset audioClip;
        bool playOnAwake = true;
        bool muted = false;
        bool loop = false;
        float volume = 1;
        float pitch = 1;

        /* --------------------------------------------------------------------------- */
        /* Non-Serialised Members                                                      */
        /* --------------------------------------------------------------------------- */

        FMOD::Channel* channel = nullptr;
        unsigned int loopBegin = 0;
        unsigned int loopEnd = 0;
    };
}
