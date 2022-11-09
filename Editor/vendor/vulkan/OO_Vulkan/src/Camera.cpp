/************************************************************************************//*!
\file           Camera.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Camera class

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "Camera.h"
#include <iostream>
#include <algorithm> // std min


#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

void Camera::UpdateViewMatrixQuaternion()
{
	//pitch (rot around x in radians), 
	//yaw (rot around y in radians), 
	//roll (rot around z in radians)

	////FPS camera:  RotationX(pitch) * RotationY(yaw)
	//glm::quat qPitch	= glm::angleAxis(glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
	//glm::quat qYaw	= glm::angleAxis(glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
	//glm::quat qRoll	= glm::angleAxis(glm::radians(m_rotation.z), glm::vec3(0, 0, 1));

	////For a FPS camera we can omit roll
	//glm::quat orientation = qPitch * qYaw;
	//glm::quat orientation = m_orientation;

	//orientation = glm::normalize(orientation);
	//glm::mat4 rotate = glm::mat4_cast(orientation);

	//glm::mat4 translate = glm::mat4(1.0f);
	//translate = glm::translate(translate, -m_position);

	//m_right		= glm::rotate(glm::inverse(orientation), glm::vec3{ 1, 0, 0 });
	//m_forward	= glm::rotate(glm::inverse(orientation), glm::vec3{ 0, 0, -1 });
	////m_up		= glm::rotate(orientation, glm::vec3{ 0, 1, 0 });

	//glm::vec3 worldUp{ 0, 1, 0 };
	////if(m_CameraMovementType == CameraMovementType::lookat)
	////{
	////	/*glm::vec3 fromTarget{ 0.0f, 0.0f, -m_TargetDistance };
	////	fromTarget = glm::mat3(rotM) * fromTarget;
	////	m_position = m_TargetPosition + fromTarget;
	////	m_forward = glm::normalize(m_TargetPosition - m_position);
	////	worldUp = glm::eulerAngleZ(glm::radians(m_rotation.z)) * glm::vec4(worldUp,0.0f);*/
	////	//worldUp = glm::eulerAngleZ(orientation.z) * glm::vec4{worldUp, 0.0f};
	////}
	////else
	////{
	////	m_forward = rotate * glm::vec4{ 0.0f, 0.0f, 1.0f ,0.0f };
	////	//m_forward = rotM * glm::vec4{ 0.0f, 0.0f, 1.0f ,0.0f };
	////}
	//
	//// vulkan uses right-handed coordinate system.
	//matrices.view = glm::lookAtRH(m_position, m_position + m_forward, worldUp);

	//FPS camera:  RotationX(pitch) * RotationY(yaw)
	//glm::quat qPitch = glm::angleAxis(glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
	//glm::quat qYaw = glm::angleAxis(glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
	//glm::quat qRoll	= glm::angleAxis(glm::radians(m_rotation.z), glm::vec3(0, 0, 1));

	////For a FPS camera we can omit roll
	glm::quat orientation = m_orientation; //qPitch * qYaw; // *qRoll;
	orientation = glm::normalize(orientation);
	glm::mat4 rotate = glm::mat4_cast(orientation);

	// calculate translation matrix
	glm::mat4 translate = glm::mat4(1.0f);
	translate = glm::translate(translate, m_position);

	// we inverse because the orientation is the camera's transform. view = inverse(camera_transform)
	auto view_orientation = orientation; //glm::inverse(orientation);

	m_right = glm::rotate(view_orientation, glm::vec3{ 1, 0, 0 });
	m_forward = glm::rotate(view_orientation, glm::vec3{ 0, 0, 1 });
	//m_up = glm::rotate(view_orientation, glm::vec3{ 0, 1, 0 });
	
	matrices.view = glm::lookAt(m_position, m_position + m_forward, { 0, 1, 0}); // = glm::inverse(translate * rotate);
	updated = true;
}

//void Camera::RotatePitch(float rads) // rotate around cams local X axis
//{
//	glm::quat qPitch = glm::angleAxis(rads, glm::vec3(1, 0, 0));
//
//	m_orientation = glm::normalize(qPitch) * m_orientation;
//	glm::mat4 rotate = glm::mat4_cast(m_orientation);
//
//	glm::mat4 translate = glm::mat4(1.0f);
//	translate = glm::translate(translate, -m_position);
//
//	//matrices.view = rotate * translate;
//}
//
//void Camera::RotateYaw(float rads)
//{
//	glm::quat qYaw = glm::angleAxis(rads, glm::vec3(0, 1, 0));
//
//	m_orientation = glm::normalize(qYaw) * m_orientation;
//	glm::mat4 rotate = glm::mat4_cast(m_orientation);
//
//	glm::mat4 translate = glm::mat4(1.0f);
//	translate = glm::translate(translate, -m_position);
//
//	//matrices.view = rotate * translate;
//}

//void Camera::RotateRoll(float rads)
//{
//	glm::quat qRoll = glm::angleAxis(rads, glm::vec3(0, 0, 1));
//
//	m_orientation = glm::normalize(qRoll) * m_orientation;
//	glm::mat4 rotate = glm::mat4_cast(m_orientation);
//
//	glm::mat4 translate = glm::mat4(1.0f);
//	translate = glm::translate(translate, -m_position);
//
//	matrices.view = rotate * translate;
//}

//void Camera::RotateAll(glm::vec3 deltaRads)
//{
//	RotatePitch(deltaRads.x);
//	RotateYaw(deltaRads.y);
//	RotateRoll(deltaRads.z);
//}

//void Camera::Update(float deltaTimeSeconds)
//{
//	//FPS camera:  RotationX(pitch) * RotationY(yaw)
//	glm::quat qPitch = glm::angleAxis(m_d_pitch, glm::vec3(1, 0, 0));
//	glm::quat qYaw = glm::angleAxis(m_d_yaw, glm::vec3(0, 1, 0));
//	glm::quat qRoll = glm::angleAxis(m_d_roll, glm::vec3(0, 0, 1));
//
//	//For a FPS camera we can omit roll
//	glm::quat m_d_orientation = qPitch * qYaw;
//	glm::quat delta = glm::mix(glm::quat(0, 0, 0, 0), m_d_orientation, deltaTimeSeconds);
//	m_orientation = glm::normalize(delta) * m_orientation;
//	glm::mat4 rotate = glm::mat4_cast(orientation);
//
//	glm::mat4 translate = glm::mat4(1.0f);
//	translate = glm::translate(translate, -eye);
//
//	viewMatrix = rotate * translate;
//}

//void Camera::updateViewMatrix()
//{
//	m_rotation.x = glm::clamp(m_rotation.x, -89.0f, 89.0f);
//	glm::mat4 rotM = glm::eulerAngleYXZ(glm::radians(m_rotation.y),glm::radians(m_rotation.x), glm::radians(m_rotation.z));
//
//	glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
//	if (m_CameraMovementType == CameraMovementType::lookat)
//	{
//		glm::vec3 fromTarget{ 0.0f, 0.0f, -m_TargetDistance };
//		fromTarget = glm::mat3(rotM) * fromTarget;
//		m_position = m_TargetPosition + fromTarget;
//		m_forward = glm::normalize(m_TargetPosition - m_position);
//		worldUp = glm::eulerAngleZ(glm::radians(m_rotation.z)) * glm::vec4(worldUp,0.0f);
//	}
//	else
//	{
//		m_forward = rotM * glm::vec4{ 0.0f, 0.0f, 1.0f ,0.0f };
//	}
//		
//	m_right = glm::cross(worldUp,m_forward);
//	m_up = glm::cross(m_forward,m_right);
//
//	m_forward = glm::normalize(m_forward);
//	m_right = glm::normalize(m_right);
//	m_up = glm::normalize(m_up);
//
//	// Just use GLM... Tested by the whole world...
//	matrices.view = glm::lookAtRH(m_position, m_position + m_forward, worldUp);
//	updated = true;
//}

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
	UpdateViewMatrixQuaternion();

	//updateViewMatrix();
}

//void Camera::SetRotation(glm::vec3 rotation)
//{
//	this->m_rotation = rotation;
//	updateViewMatrix();
//}

void Camera::SetRotation(glm::quat orientation)
{
	this->m_orientation = orientation;
	UpdateViewMatrixQuaternion();
	m_rotation = glm::eulerAngles(orientation);
}

void Camera::Rotate(glm::vec3 delta)
{
	m_rotation += delta;
	m_rotation.x = glm::clamp(m_rotation.x, -89.0f, 89.0f);	// clamp x value.

	auto rotation_rads		= glm::radians(m_rotation);
	glm::quat qPitch		= glm::angleAxis(rotation_rads.x, glm::vec3(1, 0, 0));
	glm::quat qYaw			= glm::angleAxis(rotation_rads.y, glm::vec3(0, 1, 0));
	//glm::quat qRoll		= glm::angleAxis(glm::radians(delta.z), glm::vec3(0, 0, 1));

	m_orientation = glm::normalize(qYaw) * glm::normalize(qPitch) /** glm::normalize(qRoll) */ * glm::quat{ 0, 0, 0, 1 };
	UpdateViewMatrixQuaternion();
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
	UpdateViewMatrixQuaternion();
	//updateViewMatrix();
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
	UpdateViewMatrixQuaternion();
	//updateViewMatrix();
}

void Camera::ChangeTargetDistance(float delta)
{
	m_TargetDistance = std::max(1.0f, delta + m_TargetDistance);
	UpdateViewMatrixQuaternion();
	//updateViewMatrix();
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
			//m_rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			//auto val = pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			//RotateYaw(glm::radians(val));
			retVal = true;
		}
		if (fabsf(axisRight.y) > deadZone)
		{
			float pos = (fabsf(axisRight.y) - deadZone) / range;
			//m_rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			//auto val = -(pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed);
			//RotateYaw(glm::radians(val));
			retVal = true;
		}
		
	}
	else
	{
		// todo: move code from example base class for look-at
	}

	if (retVal)
	{
		UpdateViewMatrixQuaternion();
		//updateViewMatrix();
	}

	return retVal;
}

void Camera::UpdateProjectionMatrix()
{
	if (m_aspectRatio != m_aspectRatio)
	{ 
		//assert(false && "Times like this we must ask ourselves - why is aspect ratio NaN");
		return;
	}
	assert(m_aspectRatio != 0.0f);

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
