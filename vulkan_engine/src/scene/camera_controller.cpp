#include "camera_controller.h"
#include "core/input.h"
#include "components.h"
#include "utils/math.h"

void CameraController::onCreate()
{
}

void CameraController::onUpdate()
{
	auto& camera = getComponent<CameraComponent>();
	auto& transform = getComponent<TransformComponent>();
	const glm::vec3 position = math::position(transform);
	camera.projection_view = glm::perspective(glm::radians(80.0f), 1.777f, 0.01f, 1000.0f) * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), math::UP);
}

void CameraController::onDestroy()
{
}
