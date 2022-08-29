#include "Camera.h"
#include <iostream>
#include <algorithm> // std min


void Camera::updateViewMatrix()
{

	rotation.x = std::max(rotation.x, -89.0f);
	rotation.x = std::min(rotation.x,  89.0f);


	glm::mat4 rotM = glm::eulerAngleYXZ(glm::radians(rotation.y),glm::radians(rotation.x), glm::radians(rotation.z));
	

	glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
	if (type == CameraType::lookat)
	{
		glm::vec3 from{ 0.0f,0.0f, -distance };
		glm::vec3 temp = glm::mat3(rotM) * from;
		from = temp;
		position = target + from;
		m_forward = position- target;
		worldUp = glm::eulerAngleZ(glm::radians(rotation.z)) * glm::vec4(worldUp,0.0f);
	}
	else
	{
		m_forward = rotM * glm::vec4{ 0.0f,0.0f,-1.0f ,0.0f };
	}

		
	m_right = glm::cross(worldUp,m_forward);
	m_up = glm::cross(m_forward,m_right);

	m_forward = glm::normalize(m_forward);
	m_right = glm::normalize(m_right);
	m_up = glm::normalize(m_up);

	matrices.view = glm::mat4{
	 m_right.x,  m_up.x,  m_forward.x,  0.0f,
	 m_right.y,  m_up.y,  m_forward.y,  0.0f,
	 m_right.z,  m_up.z,  m_forward.z,  0.0f,
	 -glm::dot(position,m_right),   -glm::dot(position,m_up),   -glm::dot(position,m_forward),  1.0f
	};
	
	updated = true;
}

bool Camera::Moving()
{
	return keys.left || keys.right || keys.up || keys.down;
}

float Camera::GetNearClip() 
{
	return znear;
}

float Camera::GetFarClip() 
{
	return zfar;
}

void Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
	this->fov = fov;
	this->znear = znear;
	this->zfar = zfar;
	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	if (flipY) {
		matrices.perspective[1][1] *= -1.0f;
	}
}

void Camera::LookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& upVec)
{
	LookAtDirection(pos, target - pos, upVec);
}

void Camera::SetOrtho(float size, float aspect, float znear, float zfar)
{
	this->fov = fov;
	this->znear = znear;
	this->zfar = zfar;
	auto h = size / aspect;
	matrices.perspective = glm::ortho(-size,size,-h,h,znear,zfar);
	if (flipY) {
		matrices.perspective[1][1] *= -1.0f;
	}
}

void Camera::LookAtDirection(const glm::vec3& pos, const glm::vec3& direction, const glm::vec3& upVec)
{
	// Invalid, ignore
	if (glm::dot(direction,direction) <= EPSILON * EPSILON)
		return;

	// Save eye position
	position = pos;

	// Calculate matrix
	glm::vec3 view = glm::normalize(-direction);
	glm::vec3 r = glm::normalize(glm::cross(view, upVec));
	glm::vec3 up = glm::cross(view ,r);

	matrices.view = glm::mat4{
		r.x,  up.x,  view.x,  0.0f,
		r.y,  up.y,  view.y,  0.0f,
		r.z,  up.z,  view.z,  0.0f,
		-glm::dot(position,r),   -glm::dot(position,up),   -glm::dot(position,view),  1.0f
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

	position =
	{
		target.x + distance * COS_VERT * COS_HORI,
		target.y + distance * SIN_VERT,
		target.z + distance * COS_VERT * SIN_HORI
	};

	// Calculate Up
	LookAt(position, target);
}


void Camera::UpdateAspectRatio(float aspect)
{
	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	if (flipY) {
		matrices.perspective[1][1] *= -1.0f;
	}
}

void Camera::SetPosition(glm::vec3 position)
{
	this->position = position;
	updateViewMatrix();
}

void Camera::SetRotation(glm::vec3 rotation)
{
	this->rotation = rotation;
	updateViewMatrix();
}

void Camera::Rotate(glm::vec3 delta)
{
	this->rotation += delta;
	updateViewMatrix();
}

void Camera::SetTranslation(glm::vec3 translation)
{
	if(type == Camera::CameraType::lookat)
	{
		this->target = translation;
	}
	else
	{
		this->position = translation;
	}
	updateViewMatrix();
}

void Camera::Translate(glm::vec3 delta)
{
	if(type == Camera::CameraType::lookat)
	{
		this->target += delta;
	}
	else
	{
		this->position += delta;
	}
	updateViewMatrix();
}

void Camera::SetRotationSpeed(float rotationSpeed)
{
	this->rotationSpeed = rotationSpeed;
}

void Camera::SetMovementSpeed(float movementSpeed)
{
	this->movementSpeed = movementSpeed;
}

void Camera::ChangeDistance(float delta)
{
	distance = std::max(1.0f, delta + distance);
	updateViewMatrix();
}

void Camera::Update(float deltaTime)
{
	updated = false;
	if (type == CameraType::firstperson)
	{
		if (Moving())
		{
			
			float moveSpeed = deltaTime * movementSpeed;

			if (keys.up)
				position -= m_forward * moveSpeed;
			if (keys.down)
				position += m_forward * moveSpeed;
			if (keys.left)
				position -= m_right * moveSpeed;
			if (keys.right)
				position += m_right * moveSpeed;

			//updateViewMatrix();
		}
	}
}
glm::vec3 Camera::GetFront()
{
	return m_forward;
}
glm::vec3 Camera::GetRight()
{
	return m_right;
}
glm::vec3 Camera::GetUp()
{
	return m_up;
}


bool Camera::UpdatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)

{
	bool retVal = false;

	if (type == CameraType::firstperson)
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
			position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
			retVal = true;
		}
		if (fabsf(axisLeft.x) > deadZone)
		{
			float pos = (fabsf(axisLeft.x) - deadZone) / range;
			position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
			retVal = true;
		}

		// Rotate
		if (fabsf(axisRight.x) > deadZone)
		{
			float pos = (fabsf(axisRight.x) - deadZone) / range;
			rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			retVal = true;
		}
		if (fabsf(axisRight.y) > deadZone)
		{
			float pos = (fabsf(axisRight.y) - deadZone) / range;
			rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
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
