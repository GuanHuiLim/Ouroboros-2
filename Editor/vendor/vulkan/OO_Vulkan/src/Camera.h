#pragma once

#include "MathCommon.h"

class Camera
{
private:
	static constexpr float EPSILON{ 0.001f };

	glm::vec3 m_forward{ 0.0f, 0.0f, -1.0f };
	glm::vec3 m_up{ 0.0f,1.0f,0.0f };
	glm::vec3 m_right{ 1.0f, 0.0f, 0.0f };

    float m_fovDegrees{ 60.0f };
	float m_aspectRatio{ 1.0f };
	float m_orthoSize{ 10.0f };
	float m_znear{ 0.1f };
    float m_zfar{ 10000.0f };

	void updateViewMatrix();
public:
	enum class CameraMovementType { lookat, firstperson };
	enum class CameraProjectionType { perspective, orthographic };
	CameraMovementType m_CameraMovementType{ CameraMovementType::lookat };
	CameraProjectionType m_CameraProjectionType{ CameraProjectionType::perspective };

	glm::vec3 m_rotation{};
	glm::vec3 m_position{};

	glm::vec3 m_TargetPosition{ 0.0f, 0.0f, 0.0f };
	float m_TargetDistance{ 10.0f };

	float rotationSpeed{ 1.0f };
	float movementSpeed{ 1.0f };

	bool updated{ false };
	bool flipY{ false };

	struct
	{
		glm::mat4 perspective{};
		glm::mat4 view{};
	} matrices{};

	struct
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
	} keys;

	bool Moving() const { return keys.left || keys.right || keys.up || keys.down; };
	float GetNearClip() const { return m_znear; };
	float GetFarClip() const { return m_zfar; };

	void LookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& upVec = {0.0f,1.0f,0.0f});

	void LookAtDirection(const glm::vec3& pos, const glm::vec3& direction, const glm::vec3& upVec = {0.0f,1.0f,0.0f});

	void LookFromAngle(float distance, const glm::vec3& target, float vertAngle, float horiAngle);

	void SetPosition(glm::vec3 position);

	void SetRotation(glm::vec3 rotation);

	void Rotate(glm::vec3 delta);

	void SetTranslation(glm::vec3 translation);

	void Translate(glm::vec3 delta);

	void SetRotationSpeed(float rotationSpeed) { this->rotationSpeed = rotationSpeed; };
	void SetMovementSpeed(float movementSpeed) { this->movementSpeed = movementSpeed; };

	void ChangeTargetDistance(float delta);

	glm::vec3 GetFront() const { return m_forward; }
	glm::vec3 GetRight() const { return m_right; };
	glm::vec3 GetUp() const { return m_up; };

	// Update camera passing separate axis data (gamepad)
	// Returns true if view or position has been changed
	bool UpdatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime);

	void SetAspectRatio(float aspect) { m_aspectRatio = aspect; }

	void UpdateProjectionMatrix();

	// 
	bool m_ViewMatrixOutdated{ true };
	bool m_ProjectionMatrixOutdated{ true };
};