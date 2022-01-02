#pragma once

class Layer
{
public:
    Layer(const std::string &name = "Layer");
    virtual ~Layer() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate() {}
    virtual void onImGuiRender() {}

    const std::string &getName() const { return debugName; }

protected:
    std::string debugName;
};