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

#include "MathCommon.h"

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
	m_up = -glm::cross(m_right, m_forward);
	
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

void Camera::SetJitterValues(glm::vec2 vals)
{
	jitterValues = vals;
	SetDirty();
}

oGFX::Frustum Camera::GetFrustum() const
{
	oGFX::Frustum frustum;

	glm::vec3 near_center = m_position + m_forward * m_znear;
	glm::vec3 far_center = m_position + m_forward * m_zfar;

	float near_half_height = tan(glm::radians(m_fovDegrees )/ 2) * m_znear;
	float near_half_width = near_half_height * m_aspectRatio;
	float far_half_height = tan(glm::radians(m_fovDegrees )/ 2) * m_zfar;
	float far_half_width = far_half_height * m_aspectRatio;

	glm::vec3 ntl = near_center + m_up * near_half_height - m_right * near_half_width;
	glm::vec3 ntr = near_center + m_up * near_half_height + m_right * near_half_width;
	glm::vec3 nbl = near_center - m_up * near_half_height - m_right * near_half_width;
	glm::vec3 nbr = near_center - m_up * near_half_height + m_right * near_half_width;
	glm::vec3 ftl = far_center  + m_up * far_half_height  - m_right * far_half_width;
	glm::vec3 ftr = far_center  + m_up * far_half_height  + m_right * far_half_width;
	glm::vec3 fbl = far_center  - m_up * far_half_height  - m_right * far_half_width;
	glm::vec3 fbr = far_center  - m_up * far_half_height  + m_right * far_half_width;

	glm::vec3 near_plane =	 -glm::normalize(glm::cross(ntr - ntl, ntl - nbl));
	glm::vec3 far_plane =	 glm::normalize(glm::cross(ftr - ftl, ftl - fbl));
	glm::vec3 left_plane =	 -glm::normalize(glm::cross(ntl - fbl, ftl - ntl));
	glm::vec3 right_plane =  glm::normalize(glm::cross(nbr - fbr, ftr - fbr));
	glm::vec3 top_plane =	 -glm::normalize(glm::cross(ntr - ftr, ftr - ntl));
	glm::vec3 bottom_plane = -glm::normalize(glm::cross(nbl - fbl, fbl - nbr));

	frustum.planeNear = { near_plane,		glm::dot(near_plane, ntl) };
	frustum.planeFar =	{ far_plane,		glm::dot(far_plane, ftl) };
	frustum.left =		{ left_plane,		glm::dot(left_plane, ntl) };
	frustum.right =		{ right_plane,		glm::dot(right_plane, nbr) };
	frustum.top =		{ top_plane,		glm::dot(top_plane, ntr) };
	frustum.bottom =	{ bottom_plane,		glm::dot(bottom_plane, nbl) };
	
	//frust.right.normal.w = glm::dot(glm::vec3{ frust.right.normal }, start);
	
	glm::vec3 act_centre = m_position + m_forward * ((m_zfar - m_znear)/2.0f);
	act_centre = m_position + m_forward * 5.0f; // hardcoded
	float centre_half_height = tan(glm::radians(m_fovDegrees)/ 2) * 5.0f;
	float centre_half_width = centre_half_height * m_aspectRatio;

	frustum.pt_left  = act_centre - centre_half_width * m_right;
	frustum.pt_right = act_centre + centre_half_width * m_right;
	frustum.pt_top = act_centre + centre_half_height * m_up;
	frustum.pt_bottom = act_centre - centre_half_height * m_up;
	frustum.pt_planeNear = near_center;
	frustum.pt_planeFar = far_center;

	return frustum;
}

void Camera::LookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& upVec)
{
	//throw; why throw..
	__debugbreak();
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
	//updateViewMatrix();
}

void Camera::ChangeTargetDistance(float delta)
{
	m_TargetDistance = std::max(1.0f, delta + m_TargetDistance);
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
		//updateViewMatrix();
	}

	return retVal;
}

glm::mat4 inversed_infinite_perspectiveRH_ZO(float fovRad, float aspect, float n, float f) {
	glm::mat4 result(0.0f);
	assert(abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
	float const tanHalfFovy = tan(fovRad / 2.0f);

	float h = 1.0f / std::tan(fovRad * 0.5f);
	float w = h / aspect;
	float a = -n / (f - n);
	float b = (n * f) / (f - n);
	result[0][0] = w;
	result[1][1] = h;
	result[2][2] = a;
	result[2][3] = -1.0f;
	result[3][2] = b;
	
	result[2][2] = 0.0f; //infinite
	result[3][2] = n; //infinite

	return result;
};


void Camera::SetDirty()
{
	m_ProjectionMatrixOutdated = true;
}

glm::mat4 Camera::GetNonInvProjectionMatrix()
{
	return glm::perspective(glm::radians(m_fovDegrees), m_aspectRatio, m_znear, m_zfar);
}

void Camera::UpdateMatrices()
{
	// Always update temporal information
	previousMat.perspective = matrices.perspective;
	previousMat.perspectiveJittered = matrices.perspectiveJittered;
	previousMat.view = matrices.view;


	UpdateViewMatrixQuaternion();

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
#define INVERSED_DEPTH
#ifdef INVERSED_DEPTH

		matrices.perspective = inversed_infinite_perspectiveRH_ZO(glm::radians(m_fovDegrees), m_aspectRatio, m_znear, m_zfar);
		glm::mat4 jitterTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(jitterValues, 0.0f));
		matrices.perspectiveJittered = jitterTranslation * matrices.perspective;
#else
		matrices.perspective = glm::perspective(glm::radians(m_fovDegrees), m_aspectRatio, m_znear, m_zfar);

#endif // INVERSED_DEPTH

	}
}
