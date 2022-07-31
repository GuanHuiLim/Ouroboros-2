/************************************************************************************//*!
\file           TestScene.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes a basic empty test scene for debugging purposes

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <SceneManagement/include/IScene.h>

struct TestScene : public IScene
{
    virtual void Init() override { }
    virtual void Update() override {}
    virtual void LateUpdate() override {}
    virtual void Render() override {}
    virtual void EndOfFrameUpdate() override {}
    virtual void Exit() override {}

    virtual void LoadScene() override {}
    virtual void UnloadScene() override {}
    virtual void ReloadScene() override {}

    virtual LoadStatus GetProgress() const override { return LoadStatus{}; }
};
