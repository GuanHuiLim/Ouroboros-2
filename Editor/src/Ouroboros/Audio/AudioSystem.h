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

#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"

#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

namespace oo
{
    class AudioSystem final : public Ecs::System
    {

    public:

        /* --------------------------------------------------------------------------- */
        /* Constructors and Destructors                                                */
        /* --------------------------------------------------------------------------- */

        AudioSystem(Scene* scene) : scene{ scene } {}
        virtual ~AudioSystem() = default;

        /* --------------------------------------------------------------------------- */
        /* Functions                                                                   */
        /* --------------------------------------------------------------------------- */

        virtual void Run(Ecs::ECSWorld* world) override;

    private:
        /* --------------------------------------------------------------------------- */
        /* Members                                                                     */
        /* --------------------------------------------------------------------------- */

        Scene* scene = nullptr;
    };
}
