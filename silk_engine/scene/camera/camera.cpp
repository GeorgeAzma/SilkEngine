#include "camera.h"
#include "silk_engine/utils/math.h"
#include "silk_engine/gfx/window/window.h"

Camera::Camera()
{
	direction = math::FRONT;
	if (type == Camera::Type::PERSPECTIVE)
	{
		near = 0.05f;
		far = 10000.0f;
	}
	else
	{
		near = 0.0f;
		far = 1.0f;
	}
	onViewportResize();
}

void Camera::onViewportResize()
{
	if (Window::get().isMinimized())
		return;

	if (type == Camera::Type::PERSPECTIVE)
		projection = math::perspective(math::radians(fov), Window::get().getAspectRatio(), near, far);
	else
		projection = math::ortho(0.0f, (float)Window::get().getWidth(), 0.0f, (float)Window::get().getHeight(), near, far);

	updateProjectionView();
}

void Camera::setFOV(float fov)
{
	this->fov = fov;
	onViewportResize();
}

void Camera::setNear(float near)
{
	this->near = near;
	onViewportResize();
}

void Camera::setFar(float far)
{
	this->far = far;
	onViewportResize();
}

void Camera::updateProjectionView()
{
	view = math::lookAt(position, position + direction, math::UP);
	projection_view = projection * view;
	frustum.calculatePlanes(projection_view);
}
