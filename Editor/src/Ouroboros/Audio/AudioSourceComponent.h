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
        [[nodiscard]] inline bool IsMuted() const { return muted; }
        [[nodiscard]] inline bool IsPlayOnAwake() const { return playOnAwake; }
        [[nodiscard]] inline bool IsLoop() const { return loop; }
        [[nodiscard]] inline float GetVolume() const { return volume; }
        [[nodiscard]] inline float GetPitch() const { return pitch; }
        [[nodiscard]] inline FMOD::Channel* GetChannel() const { return channel; }
        [[nodiscard]] inline bool IsDirty() const { return isDirty; }
        [[nodiscard]] bool IsPlaying() const;
        [[nodiscard]] float GetPlaybackTime() const;

        /* --------------------------------------------------------------------------- */
        /* Setters                                                                     */
        /* --------------------------------------------------------------------------- */

        void SetAudioClip(Asset a);
        void SetMuted(bool m);
        void SetPlayOnAwake(bool p);
        void SetLoop(bool l);
        void SetVolume(float v);
        void SetPitch(float p);

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

        /// <summary>
        /// Clears the dirty flag.
        /// </summary>
        void ClearDirty();

        RTTR_ENABLE();

    private:
        /* --------------------------------------------------------------------------- */
        /* Serialised Members                                                          */
        /* --------------------------------------------------------------------------- */

        Asset audioClip;
        bool muted = false;
        bool playOnAwake = true;
        bool loop = false;
        float volume = 1;
        float pitch = 1;

        /* --------------------------------------------------------------------------- */
        /* Non-Serialised Members                                                      */
        /* --------------------------------------------------------------------------- */

        FMOD::Channel* channel = nullptr;
        bool isDirty = false;
    };
}
