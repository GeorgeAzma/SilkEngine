#include "camera.h"
#include "utils/math.h"
#include "gfx/window/window.h"
#undef near
#undef far

Camera::Camera()
{
	direction = math::eulerToDirection(rotation);
	view = glm::lookAt(position, position + direction, math::UP);
	onViewportResize();
}

void Camera::onViewportResize()
{
	switch (type)
	{
	case CameraType::PERSPECTIVE:
		projection = glm::perspective(glm::radians(fov), Window::getAspectRatio(), near, far);
		break;
	case CameraType::ORTHOGRAPHIC:
		projection = glm::ortho(0.0f, (float)Window::getWidth(), 0.0f, (float)Window::getHeight(), 0.0f, 1.0f);
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
	SK_ASSERT(type == CameraType::PERSPECTIVE, "Only perspective cameras have near variable");
	this->near = near;
	projection = glm::perspective(glm::radians(fov), (float)Window::getAspectRatio(), near, far);
	updateProjectionView();
}

void Camera::setFar(float far)
{
	SK_ASSERT(type == CameraType::PERSPECTIVE, "Only perspective cameras have near variable");
	this->far = far;
	projection = glm::perspective(glm::radians(fov), (float)Window::getAspectRatio(), near, far);
	updateProjectionView();
}

void Camera::updateProjectionView()
{
	projection_view = projection * view;
	frustum.calculatePlanes(projection_view);
}
