#include "Sandbox.h"

Instance* instance = nullptr;
Sandbox::Sandbox() : Layer("Sandbox")
{
    VE_INFO("TEST");
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
