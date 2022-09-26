#include "simulation.h"
#include "core/input/input.h"
#include "scene/meshes/circle_outline_mesh.h"
#include "core/input/mouse_buttons.h"
#include "gfx/images/image.h"
#include "gfx/renderer.h"
#include "scene/resources.h"
#include "gates/and.h"
#include "gfx/window/window.h"

/*
TODO: 
- GUI
- SAVING SLOTS/SAVE GUI
- SELECTION BOX
- MERGING LOGIC GATES
*/

void Simulation::init()
{
	Resources::add<Image>("Not", makeShared<Image>("not.png"));
	Resources::add<Image>("And", makeShared<Image>("and.png"));
	Resources::add<Image>("Nand", makeShared<Image>("nand.png"));
	Resources::add<Image>("Or", makeShared<Image>("or.png"));
	Resources::add<Image>("Nor", makeShared<Image>("nor.png"));
	Resources::add<Image>("Xor", makeShared<Image>("xor.png"));
	Resources::add<Image>("Xnor", makeShared<Image>("xnor.png"));
	Resources::add<Image>("0", makeShared<Image>("0.png"));
	Resources::add<Image>("1", makeShared<Image>("1.png"));
	Resources::add<Mesh>("Pin", makeShared<Mesh>(CircleOutlineMesh(64, 0.5f)));
	Dispatcher::subscribe(&Simulation::onMousePress);
	Dispatcher::subscribe(&Simulation::onMouseRelease);
	Dispatcher::subscribe(&Simulation::onMouseScroll);

	std::filesystem::create_directories("res/saves");
	load();
}

void Simulation::update()
{
	if (selected_pin)
	{
		vec2 p = selected_pin.gate->getPinLocation(selected_pin.index, selected_pin.layer);
		Renderer::color(Colors::GRAY);
		Renderer::circle(p.x, p.y, selected_pin.gate->getPinRadius());
	}

	for (auto& gate : gates)
		gate->update();

	for (auto& gate : gates)
		gate->renderWires();

	for (auto& gate : gates)
	{
		gate->renderPins();
		gate->render();
	}

	if (moving_gate)
	{
		moving_gate->x = Input::getMouseX() + offset.x;
		moving_gate->y = Input::getMouseY() + offset.y;
	}
	else if (Input::isMouseDown(MouseButtons::MIDDLE))
	{
		offset = Input::getMouse();
		for (auto& gate : gates)
		{
			gate->x += offset.x - start_pos.x;
			gate->y += offset.y - start_pos.y;
		}
		start_pos = offset;
	}

	selection_gui.render();

	Renderer::color(Colors::WHITE);
	Renderer::text(std::format("Instances: {}", Graphics::stats.instances).c_str(), 30, Window::getHeight() - 70, 30);
	Renderer::text(std::format("Instance Batches: {}", Graphics::stats.instance_batches).c_str(), 30, Window::getHeight() - 120, 30);
}

void Simulation::onMousePress(const MousePressEvent& e)
{
	bool have_selection_pin = selected_pin;
	if (e.button == MouseButtons::LEFT)
	{
		if (moving_gate = shared<Gate>(selection_gui.onMousePress(e)))
		{
			gates.emplace_back(moving_gate);
			return;
		}
		for (auto& g : gates)
		{
			if (math::isPointInRectangle(Input::getMouse(), { g->x, g->y, g->getWidth(), g->getHeight() }))
			{
				moving_gate = g;
				moving_gate->onMousePress(e);
				start_pos = vec2(g->x, g->y);
				offset = vec2(g->x, g->y) - Input::getMouse();
				if (gates.back().get() != moving_gate.get())
					std::swap(g, gates.back());
				return;
			}
		}
		Pin destination_pin{};
		for (const auto& g : gates)
		{
			auto pin = g->getPinIndexAtLocation(Input::getMouse());	
			if (pin.first != -1)
			{
				if (!have_selection_pin)
					selected_pin = { (size_t)pin.first, pin.second, g };
				else if (selected_pin.layer != pin.second && g.get() != selected_pin.gate.get())
					destination_pin = { (size_t)pin.first, pin.second, g };
				break;
			}
		}
		if (destination_pin)
		{
			if (selected_pin.layer == 0 && destination_pin.layer == 1)
				std::swap(selected_pin, destination_pin);

			selected_pin.gate->addConnection(selected_pin.index, { destination_pin.index, destination_pin.gate });
			selected_pin = {};
			return;
		}
	}
	else if (e.button == MouseButtons::RIGHT)
	{
	}
	else if (e.button == MouseButtons::MIDDLE)
	{
		start_pos = Input::getMouse();
	}
	if (have_selection_pin)
		selected_pin = {};
}

void Simulation::onMouseRelease(const MouseReleaseEvent& e)
{
	if (e.button == MouseButtons::LEFT)
	{
		if (moving_gate)
		{
			vec2 p = vec2(moving_gate->x, moving_gate->y);
			moving_gate->moved = start_pos != p;
			moving_gate->onMouseRelease(e);
			moving_gate->moved = false;
			moving_gate = nullptr;
		}
	}
	else if (e.button == MouseButtons::RIGHT)
	{
		for (auto i = gates.rbegin(); i != gates.rend(); ++i)
		{
			if (math::isPointInRectangle(Input::getMouse(), { (*i)->x, (*i)->y, (*i)->getWidth(), (*i)->getHeight() }))
			{
				gates.erase((i + 1).base());
				break;
			}
		}
		for (auto& g : gates)
		{
			auto pin = g->getPinIndexAtLocation(Input::getMouse());
			if (pin.first != -1 && pin.second == 1)
			{
				g->clearConnections(pin.first);
				break;
			}
		}
	}
}

void Simulation::onMouseScroll(const MouseScrollEvent& e)
{
	float zoom_amount = 0.05f;
	for (auto& g : gates)
	{
		g->size += e.y * g->size * zoom_amount;
		g->x += e.y * (g->x - Input::getMouseX()) * zoom_amount;
		g->y += e.y * (g->y - Input::getMouseY()) * zoom_amount;
	}
}

void Simulation::destroy()
{
	save();
	Dispatcher::unsubscribe(&Simulation::onMouseScroll);
	Dispatcher::unsubscribe(&Simulation::onMousePress);
	Dispatcher::unsubscribe(&Simulation::onMouseRelease);
}

void Simulation::save()
{
	std::ofstream os("res/saves/save.dat", std::ios::binary);
	if (os)
	{
		for (size_t i = 0; i < gates.size(); ++i)
			gates[i]->index = i;
		size_t s = gates.size();
		os.write((const char*)&s, sizeof(s));
		for (const auto& g : gates)
			os << g;
	}
}

void Simulation::load()
{
	std::ifstream is("res/saves/save.dat", std::ios::binary);
	if (is)
	{
		size_t s = 0;
		is.read((char*)&s, sizeof(s));
		gates.resize(s);
		for (auto& g : gates)
			is >> g;
		for (auto& g : gates)
			for (auto& connection : g->connections)
				for (auto& c : connection)
					c.gate = gates[c.gate->index];
	}
}
