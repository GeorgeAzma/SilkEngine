#include "camera_controller.h"
#include "core/input/input.h"
#include "scene/components.h"
#include "utils/math.h"
#include "gfx/window/window.h"
#include "core/input/keys.h"
#include "utils/time.h"

void CameraController::onCreate()
{
	Input::lockMouse();
}

void CameraController::onUpdate()
{
	auto& camera = get<CameraComponent>().camera;

	float sensitivity = 0.003f;
	glm::vec2 dm = delta_mouse(Input::getMouse()) * sensitivity;
	dm.x *= -1;
	bool rotated = glm::length2(dm) > 0.0f;
	if (rotated)
	{
		camera.rotation += glm::vec3(dm.x, dm.y, 0.0f);
		camera.rotation.y = std::min(camera.rotation.y, glm::half_pi<float>() * 0.9999f);
		camera.rotation.y = std::max(camera.rotation.y, -glm::half_pi<float>() * 0.9999f);
		if (camera.rotation.x < -glm::pi<float>()) camera.rotation.x = glm::pi<float>();
		else if (camera.rotation.x > glm::pi<float>()) camera.rotation.x = -glm::pi<float>();
		camera.direction = math::eulerToDirection(camera.rotation);
	}
	glm::vec3 front2D(glm::normalize(glm::vec3(camera.direction.x, 0, camera.direction.z)));

	auto old_position = camera.position;
	float speed = 2.0f * Time::dt * (1 + Input::isKeyDown(Keys::LEFT_SHIFT) * 20);
	
	camera.position += float(Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S)) * front2D * speed;
	camera.position += float(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D)) * glm::cross(front2D, math::UP) * speed;
	camera.position.y += (Input::isKeyDown(Keys::SPACE) - Input::isKeyDown(Keys::LEFT_CONTROL)) * speed;

	if (old_position != camera.position || rotated)
	{
		camera.view = glm::lookAt(camera.position, camera.position + camera.direction, math::UP);
		camera.updateProjectionView();
	}
}

void CameraController::onDestroy()
{
}
