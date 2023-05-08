#include "camera_controller_2D.h"
#include "gfx/window/window.h"
#include "scene/components.h"
#include "utils/math.h"
#include "core/input/input.h"
#include "utils/time.h"

void CameraController2D::onCreate()
{
	Window::getActive().setCursorMode(CursorMode::LOCKED);
}

void CameraController2D::onUpdate()
{
	auto& camera = get<CameraComponent>().camera;

	auto old_position = camera.position;
	float speed = 2.0f * Time::dt * (1 + Input::isKeyHeld(Key::LEFT_SHIFT) * 20);
	if (Input::isKeyHeld(Key::W))
		camera.position.y += speed;
	if (Input::isKeyHeld(Key::A))
		camera.position.x -= speed;
	if (Input::isKeyHeld(Key::S))
		camera.position.y -= speed;
	if (Input::isKeyHeld(Key::D))
		camera.position.x += speed;

	bool needs_update = (old_position != camera.position);

	if (needs_update)
	{
		camera.view = math::translate(mat4(1), camera.position);
		camera.updateProjectionView();
	}
}

void CameraController2D::onDestroy()
{
}
