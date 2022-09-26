#include "layer_stack.h"

LayerStack::~LayerStack()
{
    for (Layer *layer : layers)
    {
        layer->onDetach();
        delete layer;
    }
}

void LayerStack::pushLayer(Layer *layer)
{
    layers.emplace(layers.begin() + layer_insert_index, layer);
    layer_insert_index++;
}

void LayerStack::pushOverlay(Layer *overlay)
{
    layers.emplace_back(overlay);
}

void LayerStack::popLayer(Layer *layer)
{
    auto it = std::find(layers.begin(), layers.begin() + layer_insert_index, layer);
    if (it != layers.begin() + layer_insert_index)
    {
        layer->onDetach();
        layers.erase(it);
        layer_insert_index--;
    }
}

void LayerStack::popOverlay(Layer *overlay)
{
    auto it = std::find(layers.begin() + layer_insert_index, layers.end(), overlay);
    if (it != layers.end())
    {
        overlay->onDetach();
        layers.erase(it);
    }
}