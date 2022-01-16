#include "camera_controller.h"
#include "core/input.h"
#include "components.h"
#include "utils/math.h"
#include "core/window.h"
#include "core/keys.h"
#include "core/time.h"

void CameraController::onCreate()
{
	Input::lockMouse();
}

void CameraController::onUpdate()
{
	auto& camera = getComponent<CameraComponent>();

	float sensitivity = 0.003f;
	glm::vec2 dm = delta_mouse(Input::getMouse()) * sensitivity;
	camera.rotation += glm::vec3(dm.x, dm.y, 0.0f);
	camera.rotation.y = std::min(camera.rotation.y, glm::half_pi<float>());
	camera.rotation.y = std::max(camera.rotation.y, -glm::half_pi<float>());
	if (camera.rotation.x < -glm::pi<float>()) camera.rotation.x = glm::pi<float>();
	if (camera.rotation.x > glm::pi<float>()) camera.rotation.x = -glm::pi<float>();
	camera.direction = math::eulerToDirection(camera.rotation);
	glm::vec3 front2D(glm::normalize(glm::vec3(camera.direction.x, 0, camera.direction.z)));

	float speed = 2.0f * Time::dt;
	if (Input::isKeyDown(Keys::W))
	{
		camera.position += front2D * speed;
	}
	if (Input::isKeyDown(Keys::A))
	{
		camera.position -= glm::cross(front2D, math::UP) * speed;
	}
	if (Input::isKeyDown(Keys::S))
	{
		camera.position -= front2D * speed;
	}
	if (Input::isKeyDown(Keys::D))
	{
		camera.position += glm::cross(front2D, math::UP) * speed;
	}
	if (Input::isKeyDown(Keys::SPACE))
	{
		camera.position.y -= speed;
	}
	if (Input::isKeyDown(Keys::LEFT_CONTROL))
	{
		camera.position.y += speed;
	}

	camera.view = glm::lookAt(camera.position, camera.position + camera.direction, math::UP);
	camera.projection_view = camera.projection * camera.view;
}

void CameraController::onDestroy()
{
}
