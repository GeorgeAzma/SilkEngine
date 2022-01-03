#include "Sandbox.h"

Instance* instance = nullptr;
Sandbox::Sandbox()
    : Layer("Sandbox")
{
    VE_INFO("Logging is working");
}

void Sandbox::onAttach()
{
    instance = new Instance();
}

void Sandbox::onDetach()
{
}

void Sandbox::onUpdate()
{
    if (instance)
    {
        delete instance;
        instance = nullptr;
    }
}
