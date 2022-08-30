#pragma once

#include "MathCommon.h"

class Camera
{
private:
	static constexpr float EPSILON{ 0.001f };

	float fov{ 60.0f };
	float znear{ 0.1f }, zfar{ 1000.0f };

	glm::vec3 m_forward{};
	glm::vec3 m_up{0.0f,1.0f,0.0f};
	glm::vec3 m_right{};

	void updateViewMatrix();
public:
	enum class CameraType { lookat, firstperson };
	CameraType type = CameraType::lookat;

	glm::vec3 rotation = glm::vec3{};
	glm::vec3 position = glm::vec3{};
	glm::vec4 viewPos = glm::vec4{};

	glm::vec3 target = glm::vec3{};
	float distance = 10.0f;

	float rotationSpeed = 1.0f;
	float movementSpeed = 1.0f;

	bool updated = false;
	bool flipY = false;

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

	bool Moving();

	float GetNearClip();

	float GetFarClip();

	void SetPerspective(float fov, float aspect, float znear, float zfar);
	void SetOrtho(float size, float aspect, float znear, float zfar);

	void LookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& upVec = {0.0f,1.0f,0.0f});

	void LookAtDirection(const glm::vec3& pos, const glm::vec3& direction, const glm::vec3& upVec = {0.0f,1.0f,0.0f});

	void LookFromAngle(float distance, const glm::vec3& target, float vertAngle, float horiAngle);

	void UpdateAspectRatio(float aspect);

	void SetPosition(glm::vec3 position);

	void SetRotation(glm::vec3 rotation);

	void Rotate(glm::vec3 delta);

	void SetTranslation(glm::vec3 translation);

	void Translate(glm::vec3 delta);

	void SetRotationSpeed(float rotationSpeed);

	void SetMovementSpeed(float movementSpeed);

	void ChangeDistance(float delta);

	void Update(float deltaTime);

	glm::vec3 GetFront();
	glm::vec3 GetRight();
	glm::vec3 GetUp();

	// Update camera passing separate axis data (gamepad)
	// Returns true if view or position has been changed
	bool UpdatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime);

};