#include "selection_button.h"
#include "gfx/window/window.h"
#include "core/input/mouse_buttons.h"
#include "gfx/renderer.h"
#include "scene/meshes/rounded_rectangle_mesh.h"

SelectionButton::SelectionButton(float x, float y, float width, float height)
	: x(x), y(y), width(width), height(height)
{
}

bool SelectionButton::onMousePress(const MousePressEvent& e)
{
	return math::isPointInRectangle(Window::getMouse(), { x, y, width, height });
}

void SelectionButton::render() const
{
	float add = 1.0f;
	if (math::isPointInRectangle(Window::getMouse(), { x, y, width, height }))
	{
		add = 1.2f;
		if (Window::isMouseDown(MouseButtons::LEFT))
			add *= 1.3f;
	}

	Renderer::color({ 0.5f * add, 0.5f * add, 0.5f * add, 0.15f });
	float roundness = 0.5f;
	Renderer::draw(Resources::get<GraphicsPipeline>("2D"), makeShared<Mesh>(RoundedRectangleMesh(16, roundness / (width / height), roundness)), x, y, Renderer::getActive().depth, width, height, 1);
}
