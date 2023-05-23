#pragma once

#include "frustum.h"

struct Camera
{
#undef near
#undef far
public:
	enum class Type
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

public:
	Camera();

	void onViewportResize();
	void setFOV(float fov);
	void setNear(float near);
	void setFar(float far);
	void updateProjectionView();

	Camera::Type getType() const { return type; }
	operator const mat4& () { return projection_view; }

public:
	mat4 projection_view = mat4(1);
	mat4 projection = mat4(1);
	mat4 view = mat4(1);

	vec3 position = vec3(0);
	vec3 rotation = vec3(0);
	vec3 direction = vec3(0);
	Frustum frustum;

private:
	float fov = 80.0f;
	float near = 0.05f;
	float far = 1000.0f;

	Camera::Type type = Camera::Type::PERSPECTIVE;
};