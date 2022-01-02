#pragma once
#include "layer.h"

class Sandbox : public Layer
{
public:
    Sandbox();
    void onAttach();
    void onDetach();
    void onUpdate();

private:
};