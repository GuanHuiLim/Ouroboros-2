#include "Camera.h"
#include <iostream>
#include <algorithm> // std min


void Camera::updateViewMatrix()
{
	m_rotation.x = glm::clamp(m_rotation.x, -89.0f, 89.0f);

	glm::mat4 rotM = glm::eulerAngleYXZ(glm::radians(m_rotation.y),glm::radians(m_rotation.x), glm::radians(m_rotation.z));
	
	glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
	if (m_CameraMovementType == CameraMovementType::lookat)
	{
		glm::vec3 fromTarget{ 0.0f,0.0f, -m_TargetDistance };
		fromTarget = glm::mat3(rotM) * fromTarget;
		m_position = m_TargetPosition + fromTarget;
		m_forward = glm::normalize(m_TargetPosition - m_position);
		worldUp = glm::eulerAngleZ(glm::radians(m_rotation.z)) * glm::vec4(worldUp,0.0f);
	}
	else
	{
		m_forward = rotM * glm::vec4{ 0.0f, 0.0f, 1.0f ,0.0f };
	}
		
	m_right = glm::cross(worldUp,m_forward);
	m_up = glm::cross(m_forward,m_right);

	m_forward = glm::normalize(m_forward);
	m_right = glm::normalize(m_right);
	m_up = glm::normalize(m_up);

	// Just use GLM... Tested by the whole world...
	matrices.view = glm::lookAtRH(m_position, m_position + m_forward, worldUp);

	updated = true;
}

void Camera::LookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& upVec)
{
	throw;
	LookAtDirection(pos, target - pos, upVec);
}

void Camera::LookAtDirection(const glm::vec3& pos, const glm::vec3& direction, const glm::vec3& upVec)
{
	// Invalid, ignore
	if (glm::dot(direction,direction) <= EPSILON * EPSILON)
		return;

	// Save eye position
	m_position = pos;

	// Calculate matrix
	glm::vec3 view = glm::normalize(-direction);
	glm::vec3 r = glm::normalize(glm::cross(view, upVec));
	glm::vec3 up = glm::cross(view ,r);

	matrices.view = glm::mat4{
		r.x,  up.x,  view.x,  0.0f,
		r.y,  up.y,  view.y,  0.0f,
		r.z,  up.z,  view.z,  0.0f,
		-glm::dot(m_position,r),   -glm::dot(m_position,up),   -glm::dot(m_position,view),  1.0f
	};
}

void Camera::LookFromAngle(float distance, const glm::vec3& target, float vertAngle, float horiAngle)
{
	if (distance <= EPSILON)
		return;

	// Clamp Angles
	vertAngle = std::clamp(vertAngle, glm::radians(-89.0f), glm::radians(89.0f));

	// Calculate Position
	float COS_VERT = cos(vertAngle);
	float SIN_VERT = sin(vertAngle);
	float COS_HORI = cos(horiAngle);
	float SIN_HORI = sin(horiAngle);

	m_position =
	{
		target.x + distance * COS_VERT * COS_HORI,
		target.y + distance * SIN_VERT,
		target.z + distance * COS_VERT * SIN_HORI
	};

	// Calculate Up
	LookAt(m_position, target);
}

void Camera::SetPosition(glm::vec3 position)
{
	this->m_position = position;
	updateViewMatrix();
}

void Camera::SetRotation(glm::vec3 rotation)
{
	this->m_rotation = rotation;
	updateViewMatrix();
}

void Camera::Rotate(glm::vec3 delta)
{
	this->m_rotation += delta;
	updateViewMatrix();
}

void Camera::SetTranslation(glm::vec3 translation)
{
	if(m_CameraMovementType == Camera::CameraMovementType::lookat)
	{
		this->m_TargetPosition = translation;
	}
	else
	{
		this->m_position = translation;
	}
	updateViewMatrix();
}

void Camera::Translate(glm::vec3 delta)
{
	if(m_CameraMovementType == Camera::CameraMovementType::lookat)
	{
		this->m_TargetPosition += delta;
	}
	else
	{
		this->m_position += delta;
	}
	updateViewMatrix();
}

void Camera::ChangeTargetDistance(float delta)
{
	m_TargetDistance = std::max(1.0f, delta + m_TargetDistance);
	updateViewMatrix();
}

bool Camera::UpdatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
{
	bool retVal = false;

	if (m_CameraMovementType == CameraMovementType::firstperson)
	{
		// Use the common console thumbstick layout		
		// Left = view, right = move

		const float deadZone = 0.0015f;
		const float range = 1.0f - deadZone;

		glm::vec3 camFront = GetFront();

		float moveSpeed = deltaTime * movementSpeed * 2.0f;
		float rotSpeed = deltaTime * rotationSpeed * 50.0f;

		// Move
		if (fabsf(axisLeft.y) > deadZone)
		{
			float pos = (fabsf(axisLeft.y) - deadZone) / range;
			m_position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
			retVal = true;
		}
		if (fabsf(axisLeft.x) > deadZone)
		{
			float pos = (fabsf(axisLeft.x) - deadZone) / range;
			m_position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
			retVal = true;
		}

		// Rotate
		if (fabsf(axisRight.x) > deadZone)
		{
			float pos = (fabsf(axisRight.x) - deadZone) / range;
			m_rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			retVal = true;
		}
		if (fabsf(axisRight.y) > deadZone)
		{
			float pos = (fabsf(axisRight.y) - deadZone) / range;
			m_rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			retVal = true;
		}
	}
	else
	{
		// todo: move code from example base class for look-at
	}

	if (retVal)
	{
		updateViewMatrix();
	}

	return retVal;
}

void Camera::UpdateProjectionMatrix()
{

	if (m_aspectRatio != m_aspectRatio) return;

	if (m_CameraProjectionType == CameraProjectionType::orthographic)
	{
		const float h = m_orthoSize / m_aspectRatio;
		matrices.perspective = glm::ortho(-m_orthoSize, m_orthoSize, -h, h, m_znear, m_zfar);
	}
	else
	{
		matrices.perspective = glm::perspective(glm::radians(m_fovDegrees), m_aspectRatio, m_znear, m_zfar);
	}
}
