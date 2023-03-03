#include "camera_controller.h"
#include "gfx/window/window.h"
#include "scene/components.h"
#include "utils/math.h"
#include "gfx/window/window.h"
#include "core/input/keys.h"
#include "utils/time.h"

void CameraController::onCreate()
{
}

void CameraController::onUpdate()
{
	auto& camera = get<CameraComponent>().camera;

	float sensitivity = 0.003f;
	vec2 dm = delta_mouse(Window::getActive().getMouse()) * sensitivity;
	dm.x *= -1;
	bool rotated = math::length2(dm) > 0.0f;
	if (rotated)
	{
		camera.rotation += vec3(dm.x, dm.y, 0.0f);
		camera.rotation.y = std::min(camera.rotation.y, math::half_pi<float>() * 0.9999f);
		camera.rotation.y = std::max(camera.rotation.y, -math::half_pi<float>() * 0.9999f);
		if (camera.rotation.x < -math::pi<float>()) camera.rotation.x = math::pi<float>();
		else if (camera.rotation.x > math::pi<float>()) camera.rotation.x = -math::pi<float>();
		camera.direction = math::eulerToDirection(camera.rotation);
	}
	vec3 front2D(math::normalize(vec3(camera.direction.x, 0, camera.direction.z)));

	auto old_position = camera.position;
	float speed = 2.0f * Time::dt * (1 + Window::getActive().isKeyDown(Keys::LEFT_SHIFT) * 20);
	
	camera.position += float(Window::getActive().isKeyDown(Keys::W) - Window::getActive().isKeyDown(Keys::S)) * front2D * speed;
	camera.position += float(Window::getActive().isKeyDown(Keys::A) - Window::getActive().isKeyDown(Keys::D)) * math::cross(front2D, math::UP) * speed;
	camera.position.y += (Window::getActive().isKeyDown(Keys::SPACE) - Window::getActive().isKeyDown(Keys::LEFT_CONTROL)) * speed;

	if (old_position != camera.position || rotated)
	{
		camera.updateProjectionView();
	}
}

void CameraController::onDestroy()
{
}