#include "camera.h"
#include "utils/math.h"
#include "gfx/window/window.h"

Camera::Camera()
{
	direction = math::FRONT;
	onViewportResize();
}

void Camera::onViewportResize()
{
	if (Window::getActive().isMinimized())
		return;

	switch (type)
	{
	case Camera::Type::PERSPECTIVE:
		projection = math::perspective(math::radians(fov), Window::getActive().getAspectRatio(), near, far);
		break;
	case Camera::Type::ORTHOGRAPHIC:
		projection = math::ortho(0.0f, (float)Window::getActive().getWidth(), 0.0f, (float)Window::getActive().getHeight(), 0.0f, 1.0f);
		break;
	}

	updateProjectionView();
}

void Camera::setFOV(float fov)
{
	this->fov = fov;
	onViewportResize();
}

void Camera::setNear(float near)
{
	SK_VERIFY(type == Camera::Type::PERSPECTIVE, "Only perspective cameras have near variable");
	this->near = near;
	projection = math::perspective(math::radians(fov), Window::getActive().getAspectRatio(), near, far);
	updateProjectionView();
}

void Camera::setFar(float far)
{
	SK_VERIFY(type == Camera::Type::PERSPECTIVE, "Only perspective cameras have far variable");
	this->far = far;
	projection = math::perspective(math::radians(fov), Window::getActive().getAspectRatio(), near, far);
	updateProjectionView();
}

void Camera::updateProjectionView()
{
	view = math::lookAt(position, position + direction, math::UP);
	projection_view = projection * view;
	frustum.calculatePlanes(projection_view);
}
