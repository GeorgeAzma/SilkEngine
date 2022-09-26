#pragma once

#include "gfx/renderer.h"
#include "scene/meshes/rounded_rectangle_mesh.h"

template <typename T>
struct ID
{
    constexpr operator size_t() const { return typeid(T).hash_code(); }
};

class Gate
{
public:
    struct Connection
    {
        size_t index = 0;
        shared<Gate> gate = nullptr;
    };
    static inline constexpr Color logic_type_color = Color(0.1f, 0.6f, 0.8f, 1.0f);
    static inline constexpr Color constant_type_color = Color(1.0f, 0.1f, 0.2f, 1.0f);
    static inline constexpr Color misc_type_color = Color(0.0f, 0.8f, 0.2f, 1.0f);

public:
    Gate() {}
    Gate(int input_count, size_t output_count)
        : inputs(input_count), outputs(output_count), connections(output_count)
    {
    }

    virtual Gate* clone() const { return new Gate(*this); }
    virtual operator size_t() const { return ID<Gate>(); }

    virtual void simulateLogic() {}
    virtual void update() {}
    virtual void onMousePress(const MousePressEvent& e) {}
    virtual void onMouseRelease(const MouseReleaseEvent& e) {}
    void updateOutputs(std::unordered_set<size_t> exclude = {})
    {
        simulateLogic();
        for (size_t i = 0; i < outputs.size(); ++i)
        {
            for (auto& c : connections[i])
            {
                if (c.gate->inputs[c.index] != outputs[i])
                {
                    c.gate->inputs[c.index] = outputs[i];
                    size_t dirty_gate = (size_t)c.gate.get();
                    if (!exclude.contains(dirty_gate))
                    {
                        exclude.emplace(dirty_gate);
                        Gate& gate = *(Gate*)dirty_gate;
                        gate.updateOutputs(exclude);
                    }
                }
            }
        }
    }

    void addConnection(size_t index, const Connection& c)
    {
        connections[index].emplace_back(c);
        updateOutputs();
    }

    void removeConnection(size_t index, size_t connection_index)
    {
        auto g = connections[index][connection_index].gate;
        intptr_t i = connections[index][connection_index].index;
        std::swap(connections[index][connection_index], connections[index].back());
        connections[index].pop_back();
        g->inputs[i] = 0;
        g->updateOutputs();
    }

    void clearConnections(size_t index)
    {
        size_t s = connections[index].size();
        for (size_t i = 0; i < s; ++i)
            removeConnection(index, 0);
    }

    friend std::ostream& operator<<(std::ostream& os, const shared<Gate>& gate)
    {
        if (!gate)
            return os;
        size_t id = size_t(*gate);
        os.write((const char*)&id, sizeof(id));
        os.write((const char*)&gate->x, sizeof(gate->x));
        os.write((const char*)&gate->y, sizeof(gate->y));
        os.write((const char*)&gate->size, sizeof(gate->size));
        size_t s = gate->inputs.size();
        os.write((const char*)&s, sizeof(s));
        os.write((const char*)gate->inputs.data(), s * sizeof(char));
        s = gate->outputs.size();
        os.write((const char*)&s, sizeof(s));
        os.write((const char*)gate->outputs.data(), s * sizeof(char));
        for (auto& connection : gate->connections)
        {
            s = connection.size();
            os.write((const char*)&s, sizeof(s));
            for (auto& c : connection)
            {
                os.write((const char*)&c.index, sizeof(c.index));
                os.write((const char*)&c.gate->index, sizeof(c.gate->index));
            }
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, shared<Gate>& gate);

public:
    std::vector<char> inputs;
    std::vector<char> outputs;
    std::vector<std::vector<Connection>> connections;



    // RENDER CODE ////////////////////////////////

public:
    virtual void render(bool as_gui = false) const {}
    float getSize() const { return size; };
    float getWidth() const { return getSize(); }
    float getHeight() const { return getSize() + getSize() * 0.2f * (std::max(inputs.size(), outputs.size()) - 1); }
    float getPinOffset() const { return getSize() * 0.3f; }
    float getPinRadius() const { return getSize() * 0.2f; }
    vec2 getPinLocation(size_t index, size_t layer) const
    {
        vec2 p = vec2(x, y);
        switch (layer)
        {
        case 0:
            p.x -= getPinOffset();
            p.y += getHeight() / (inputs.size() + 1) * (index + 1);
            break;
        case 1:
            p.x += getWidth() + getPinOffset();
            p.y += getHeight() / (outputs.size() + 1) * (index + 1);
            break;
        }
        return p;
    }

    std::pair<intptr_t, size_t> getPinIndexAtLocation(vec2 p) const
    {
        for (intptr_t i = 0; i < inputs.size(); ++i)
            if (math::isPointInCircle(p, { getPinLocation(i, 0), getPinRadius() }))
                return { i, 0 };
        for (intptr_t i = 0; i < outputs.size(); ++i)
            if (math::isPointInCircle(p, { getPinLocation(i, 1), getPinRadius() }))
                return { i, 1 };
        return { -1, 0 };
    }

    void renderGeneric(std::string_view gate_image_name, bool as_gui = true, Color color = { 0.1f, 0.6f, 0.8f }, bool circle = false) const
    {
        float width = getWidth();
        float height = getHeight();
        if (as_gui)
        {
            width = getSize();
            height = getSize();
        }
        Renderer::color({ color.r * 0.65f, color.g * 0.65f, color.b * 0.65f, color.a });
        float roundness = circle ? 1.0f : 0.5f;
        float w = width;
        float h = height;
        Renderer::draw(Resources::get<GraphicsPipeline>("2D"), makeShared<Mesh>(RoundedRectangleMesh(16, roundness / (w / h), roundness)), x, y, Renderer::getActive().depth, w, h);

        Renderer::color(color);
        float margin_x = width * 0.07f / (w / h);
        float margin_y = height * 0.07f;
        w = width - margin_x * 2.0f;
        h = height - margin_y * 2.0f;
        Renderer::draw(Resources::get<GraphicsPipeline>("2D"), makeShared<Mesh>(RoundedRectangleMesh(16, roundness / (w / h), roundness)), x + margin_x, y + margin_y, Renderer::getActive().depth, w, h);

        Renderer::color(Colors::WHITE);
        float image_margin_x = getSize() * 0.2f;
        float image_margin_y = getSize() * 0.25f;
        w = getSize() - (margin_x + image_margin_x) * 2.0f;
        h = getSize() - (margin_y + image_margin_y) * 2.0f;
        if (gate_image_name.size())
            Renderer::image(Resources::get<Image>(gate_image_name), x + margin_x + image_margin_x, y + margin_y + (height - image_margin_y * 2.0f) * 0.5f, w, h);
    }

    void renderWires()
    {
        for (size_t i = 0; i < connections.size(); ++i)
        {
            for (auto& c : connections[i])
            {
                vec2 p = getPinLocation(i, 1);
                vec2 other_p = c.gate->getPinLocation(c.index, 0);
                Renderer::color(outputs[i] ? Colors::GRAY : Colors::DARK_GRAY);
                Renderer::line(p.x, p.y, other_p.x, other_p.y, getPinRadius() * 0.5f);
                Renderer::color(Colors::GRAY);
                Renderer::circle(p.x, p.y, getPinRadius());
                Renderer::circle(other_p.x, other_p.y, getPinRadius());
            }
        }
    }

    void renderPins()
    {
        Renderer::color(Colors::DARK_GRAY);
        // Input Pins
        for (intptr_t i = 0; i < inputs.size(); ++i)
        {
            vec2 p = getPinLocation(i, 0);
            Renderer::draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Pin"), p.x, p.y, Renderer::getActive().depth, getPinRadius(), getPinRadius());
            Renderer::line(p.x + getPinRadius() * 0.8f, p.y, p.x + getPinOffset(), p.y, getPinRadius());
        }
        // Output Pins
        for (intptr_t i = 0; i < outputs.size(); ++i)
        {
            vec2 p = getPinLocation(i, 1);
            Renderer::draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Pin"), p.x, p.y, Renderer::getActive().depth, getPinRadius(), getPinRadius());
            Renderer::line(p.x - getPinOffset(), p.y, p.x - getPinRadius() * 0.8f, p.y, getPinRadius());
        }
    }

public:
    float x = 0.0f;
    float y = 0.0f;
    float size = 40.0f;
    size_t index = 0; // For saving
    bool moved = false;
};