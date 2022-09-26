#include "selection_gui.h"
#include "gfx/renderer.h"
#include "gfx/window/window.h"
#include "gates/gates.h"
#include "scene/meshes/rounded_rectangle_mesh.h"

SelectionGui::SelectionGui()
{
	gates.emplace_back(makeShared<Not>());
	gates.emplace_back(makeShared<And>());
	gates.emplace_back(makeShared<Nand>());
	gates.emplace_back(makeShared<Or>());
	gates.emplace_back(makeShared<Nor>());
	gates.emplace_back(makeShared<Xor>());
	gates.emplace_back(makeShared<Xnor>());
	gates.emplace_back(makeShared<Zero>());
	gates.emplace_back(makeShared<One>());
	gates.emplace_back(makeShared<Switch>());
	gates.emplace_back(makeShared<Button>());
	gates.emplace_back(makeShared<Bulb>());
	buttons.resize(gates.size());
}

Gate* SelectionGui::onMousePress(const MousePressEvent& e)
{
	for (size_t i = 0; i < gates.size(); ++i)
		if (buttons[i].onMousePress(e))
			return gates[i]->clone();

	return nullptr;
}

void SelectionGui::render()
{
	float w = Window::getWidth();
	float h = Window::getHeight();

	Renderer::color({ 0.4f, 0.4f, 0.4f, 0.25f });
	int rows = (h * 2) / w + 1;
	float off = 0.01f;
	float roundness = 16.0f;
	float gui_w = w * (1.0f - off * 2.0f);
	float gui_h = h * 0.1f;
	float gui_x = w * off;
	float gui_y = h * off;
	float button_off = 0.1f;
	float button_w = gui_h * (1.0f - button_off * 2.0f);
	Renderer::draw(Resources::get<GraphicsPipeline>("2D"), makeShared<Mesh>(RoundedRectangleMesh(16, roundness / gui_w, roundness / (gui_h * rows))), gui_x, gui_y, Renderer::getActive().depth, gui_w, gui_h * button_off * (rows + 1) + button_w * rows, 1);

	for (size_t i = 0; i < buttons.size(); ++i)
	{
		float off = gui_h * button_off;
		float button_x = off * (i / rows + 1) + button_w * (i / rows);
		float button_y = off * (i % rows + 1) + button_w * (i % rows);
		buttons[i] = SelectionButton(gui_x + button_x, gui_y + button_y, button_w, button_w);
		buttons[i].render();
		float gate_icon_off = 0.125f;
		gates[i]->x = buttons[i].x + gate_icon_off * button_w;
		gates[i]->y = buttons[i].y + gate_icon_off * button_w;
		float old_size = gates[i]->size;
		gates[i]->size = buttons[i].height - gate_icon_off * button_w * 2.0f;
		gates[i]->render(true);
		gates[i]->size = old_size;
	}
}
