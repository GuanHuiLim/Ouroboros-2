#pragma once

#include "OO_Vulkan/src/MathCommon.h"

#include "OO_Vulkan/src/Camera.h"

class CameraController
{
public:

    // Per tick update
    void Update(float dt, bool update_controls = true);

    // Sets a camera to control
    void SetCamera(Camera* camera) { m_Camera = camera; }

    void ShakeCamera();
    
    Camera* GetCamera() const { return m_Camera; }
private:

    glm::vec3 m_LastFrameCameraShakeOffset{ 0.0f, 0.0f, 0.0f };
    float m_CameraShakeDuration{ 0.0f };
    
    glm::vec3 m_rotation{}; //data used to clamp
    Camera* m_Camera{ nullptr };
    float m_sensitivityRate = 10;
};
