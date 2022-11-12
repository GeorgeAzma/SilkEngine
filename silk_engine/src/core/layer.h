#pragma once

class Layer
{
public:
    Layer();
    virtual ~Layer() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate() {}
};