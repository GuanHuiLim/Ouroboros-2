#pragma once
#include <IScene.h>


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
