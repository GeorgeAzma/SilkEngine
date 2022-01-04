#include "Sandbox.h"

Instance* instance = nullptr;
Sandbox::Sandbox()
    : Layer("Sandbox")
{
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
