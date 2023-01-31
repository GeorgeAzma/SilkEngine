#include "camera_controller_2D.h"
#include "gfx/window/window.h"
#include "scene/components.h"
#include "utils/math.h"
#include "gfx/window/window.h"
#include "core/input/keys.h"
#include "utils/time.h"

void CameraController2D::onCreate()
{
	Window::getActive().setCursorMode(CursorMode::LOCKED);
}

void CameraController2D::onUpdate()
{
	auto& camera = get<CameraComponent>().camera;

	auto old_position = camera.position;
	float speed = 2.0f * Time::dt * (1 + Window::getActive().isKeyDown(Keys::LEFT_SHIFT) * 20);
	if (Window::getActive().isKeyDown(Keys::W))
	{
		camera.position.y += speed;
	}
	if (Window::getActive().isKeyDown(Keys::A))
	{
		camera.position.x -= speed;
	}
	if (Window::getActive().isKeyDown(Keys::S))
	{
		camera.position.y -= speed;
	}
	if (Window::getActive().isKeyDown(Keys::D))
	{
		camera.position.x += speed;
	}

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
