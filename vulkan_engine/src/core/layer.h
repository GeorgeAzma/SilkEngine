#pragma once

class Layer
{
public:
    Layer(const char *debug_name = "Layer");
    virtual ~Layer() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate() {}

    const std::string &getName() const { return debug_name; }

protected:
    std::string debug_name;
};