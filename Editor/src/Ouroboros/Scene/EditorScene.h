/************************************************************************************//*!
\file           EditorScene.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2022
\brief          EditorScene describes the scene when its meant for editing.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Scene.h"

namespace oo
{
    class EditorScene final : public Scene
    {
    public:
        explicit EditorScene(std::string const& filepath);

        virtual void Init() override final;
        virtual void Update() override final;
        virtual void LateUpdate() override final;
        virtual void Render() override final;
        virtual void Exit() override final;

        void Save();
        void ReloadSceneWithPath(std::string const& filepath);
    };
}