/************************************************************************************//*!
\file           AudioSystem.h
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Sep 26, 2022
\brief          Contains the declaration for the Audio System.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include "App/Editor/Events/LoadSceneEvent.h"
#include "App/Editor/Events/UnloadSceneEvent.h"
#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/ECS/GameObjectComponent.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

namespace oo
{
    class AudioSystem final : public Ecs::System
    {
    public:
        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* --------------------------------------------------------------------------- */

        AudioSystem(Scene* scene);
        virtual ~AudioSystem();

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        virtual void Run(Ecs::ECSWorld* world) override;

    private:
        /* --------------------------------------------------------------------------- */
        /* Members                                                                     */
        /* --------------------------------------------------------------------------- */

        Scene* scene = nullptr;

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        /// <summary>
        /// Plays all Audio Sources marked as play on awake.
        /// </summary>
        void playAllOnAwake();

        /// <summary>
        /// Stops all Audio Sources.
        /// </summary>
        void stopAll();

        /* --------------------------------------------------------------------------- */
        /* Event Handlers                                                              */
        /* --------------------------------------------------------------------------- */

        void onLoadScene(LoadSceneEvent* e);
        void onUnloadScene(UnloadSceneEvent* e);
        void onObjectEnabled(GameObjectComponent::OnEnableEvent* e);
        void onObjectDisabled(GameObjectComponent::OnDisableEvent* e);
    };
}
