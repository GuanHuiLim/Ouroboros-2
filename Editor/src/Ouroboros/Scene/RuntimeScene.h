/************************************************************************************//*!
\file           RuntimeScene.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2021
\brief          RuntimeScene describes the scene when players are simulating the scene
                with all systems running reflective of the final build.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Scene.h"

namespace oo
{
    class RuntimeScene final : public Scene
    {
    private:
        bool m_isPause = false;
        bool m_stepMode = false;
        int m_framesLeft = 0;

    public:

        explicit RuntimeScene(std::string const& filepath);

        virtual void Init() override final;
        virtual void Update() override final;
        virtual void LateUpdate() override final;
        virtual void Render() override final;
        virtual void Exit() override final;

        void ProcessFrame(int count);
        //void StopStepMode();

        void StartSimulation();
        void StopSimulation();

        void PauseSimulation();
        void ResumeSimulation();

        bool IsPaused() const { return m_isPause; }
        bool IsStepMode() const { return m_stepMode; }
    };
}
