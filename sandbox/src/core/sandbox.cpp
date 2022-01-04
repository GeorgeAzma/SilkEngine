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
    delete instance;
}

void Sandbox::onDetach()
{
}

void Sandbox::onUpdate()
{
}
