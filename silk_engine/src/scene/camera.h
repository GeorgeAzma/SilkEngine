#pragma once

#include "frustum.h"

enum class CameraType
{
	PERSPECTIVE,
	ORTHOGRAPHIC
};

struct Camera
{
#undef near
#undef far
public:
	Camera();

	void onViewportResize();
	void setFOV(float fov);
	void setNear(float near);
	void setFar(float far);
	void updateProjectionView();

	operator const glm::mat4& () { return projection_view; }

public:
	glm::mat4 projection_view = glm::mat4(1);
	glm::mat4 projection = glm::mat4(1);
	glm::mat4 view = glm::mat4(1);

	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 direction = glm::vec3(0);
	Frustum frustum;

private:
	float fov = 80.0f;
	float near = 0.01f;
	float far = 10000.0f;

	CameraType type = CameraType::PERSPECTIVE;
};