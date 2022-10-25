#include "pch.h"

#include "CameraController.h"
#include "OO_Vulkan/src/MathCommon.h"
#include "OO_Vulkan/src/Camera.h"
#include "Input.h"

void CameraController::Update(float dt)
{
    if (m_Camera == nullptr)
        return;

    // Poll input states
    const bool strafe_right = oo::input::IsKeyHeld(KEY_D);
    const bool strafe_left  = oo::input::IsKeyHeld(KEY_A);
    const bool forward      = oo::input::IsKeyHeld(KEY_W);
    const bool backward     = oo::input::IsKeyHeld(KEY_S);
    const bool strafe_up    = oo::input::IsKeyHeld(KEY_Z);
    const bool strafe_down  = oo::input::IsKeyHeld(KEY_C);
    const auto [deltax, deltay] = oo::input::GetMouseDelta();
    const glm::vec2 mousedelta{ deltax, deltay };
    const float wheelDelta{ static_cast<float>(oo::input::GetMouseY()) };

    glm::vec3 amountToRotate = glm::vec3{ -mousedelta.y * m_Camera->rotationSpeed, -mousedelta.x * m_Camera->rotationSpeed, 0.0f };
    glm::vec3 amountToTranslate{ 0.0f, 0.0f, 0.0f };

    // Camera Movement
    if (m_Camera->m_CameraMovementType == Camera::CameraMovementType::firstperson)
    {
        const float moveSpeed = dt * m_Camera->movementSpeed;
        if (forward)
            amountToTranslate += m_Camera->GetFront() * moveSpeed;
        if (backward)
            amountToTranslate -= m_Camera->GetFront() * moveSpeed;

        if (strafe_right)
            amountToTranslate += m_Camera->GetRight() * moveSpeed;
        if (strafe_left)
            amountToTranslate -= m_Camera->GetRight() * moveSpeed;
        
        if (strafe_up)
            amountToTranslate += m_Camera->GetUp() * moveSpeed;
        if (strafe_down)
            amountToTranslate -= m_Camera->GetUp() * moveSpeed;
    }
    
    // Simulate some very simple fucked up camera shake
    if (m_CameraShakeDuration > 0.0f)
    {
        m_CameraShakeDuration -= dt;
        amountToTranslate -= m_LastFrameCameraShakeOffset;

        glm::vec3 shakeOffset{ 0.0f,0.0f,0.0f };
        float shakeAmount = 0.01f;
        shakeOffset = glm::sphericalRand(1.0f) * shakeAmount;

        if (m_CameraShakeDuration < 0.0001f)
        {
            m_CameraShakeDuration = 0.0f;
            m_LastFrameCameraShakeOffset = glm::vec3{ 0.0f,0.0f,0.0f };
        }
        else
        {
            m_LastFrameCameraShakeOffset = shakeOffset;
            amountToTranslate += shakeOffset;
        }
    }

    m_Camera->Translate(amountToTranslate);

    // Camera Rotation
    if (oo::input::IsMouseButtonHeld(MOUSE_BUTTON_RIGHT))
    {
        float post_increment_x = m_rotation.x + amountToRotate.x;
        if (post_increment_x < -89.f)   // - 100
        {
            // clamp rotation to limit
            m_rotation.x = -89.f; // -89
            // reduce induced rotation to achieve said limit internally
            amountToRotate.x = amountToRotate.x - m_rotation.x; // -100 - -89 = -11
        }
        else if (post_increment_x > 89.f) // 100
        {
            // clamp rotation to limit
            m_rotation.x = 89.f; 
            // reduce induced rotation to achieve said limit internally
            amountToRotate.x = m_rotation.x - amountToRotate.x; // 100 - 89 = 11
        }
        else
        {
            // x is within -89.f to 89.f
            m_rotation.x += amountToRotate.x;
        }

        /*m_rotation.y += amountToRotate.y;
        m_rotation.z += amountToRotate.z;*/
        m_Camera->Rotate(amountToRotate);
        LOG_TRACE("Rotate Camera amount (degrees) {0},{1}", amountToRotate.x, amountToRotate.y);
    }

    if (m_Camera->m_CameraMovementType == Camera::CameraMovementType::lookat)
    {
        // Move closer or further in the direction of the lookat target
        constexpr float targetDistanceSpeed = 0.001f;
        m_Camera->ChangeTargetDistance(-wheelDelta * targetDistanceSpeed);
    }

    // Update View & Projection Matrix
    m_Camera->UpdateProjectionMatrix();
}

void CameraController::ShakeCamera()
{
    if (m_Camera == nullptr)
        return;
    
    m_CameraShakeDuration = 0.5f;

    if (m_CameraShakeDuration < 0.0001f)
    {
        m_LastFrameCameraShakeOffset = glm::vec3{ 0.0f,0.0f,0.0f };
    }
}
