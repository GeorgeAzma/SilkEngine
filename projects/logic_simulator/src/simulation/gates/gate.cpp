#include "gates.h"

std::istream& operator>>(std::istream& is, shared<Gate>& gate)
{
    size_t id = 0;
    is.read((char*)&id, sizeof(id));
    shared<Gate> g = nullptr;
    if (id == ID<Not>())
        g = makeShared<Not>();
    else if (id == ID<And>())
        g = makeShared<And>();
    else if (id == ID<Nand>())
        g = makeShared<Nand>();
    else if (id == ID<Or>())
        g = makeShared<Or>();
    else if (id == ID<Nor>())
        g = makeShared<Nor>();
    else if (id == ID<Xor>())
        g = makeShared<Xor>();
    else if (id == ID<Xnor>())
        g = makeShared<Xnor>();
    else if (id == ID<Zero>())
        g = makeShared<Zero>();
    else if (id == ID<One>())
        g = makeShared<One>();
    else if (id == ID<Switch>())
        g = makeShared<Switch>();
    else if (id == ID<Button>())
        g = makeShared<Button>();
    else if (id == ID<Bulb>())
        g = makeShared<Bulb>();
    is.read((char*)&g->x, sizeof(g->x));
    is.read((char*)&g->y, sizeof(g->y));
    is.read((char*)&g->size, sizeof(g->size));
    size_t s = 0;
    is.read((char*)&s, sizeof(s));
    g->inputs.resize(s);
    is.read((char*)g->inputs.data(), s * sizeof(char));
    is.read((char*)&s, sizeof(s));
    g->outputs.resize(s);
    g->connections.resize(s);
    is.read((char*)g->outputs.data(), s * sizeof(char));
    for (auto& connection : g->connections)
    {
        is.read((char*)&s, sizeof(s));
        connection.resize(s);
        for (auto& c : connection)
        {
            size_t index = 0;
            is.read((char*)&index, sizeof(index));
            size_t gate_index = 0;
            is.read((char*)&gate_index, sizeof(gate_index));
            shared<Gate> gt = makeShared<Gate>();
            gt->index = gate_index;
            c = Gate::Connection{ index, gt };
        }
    }
    gate = g;
    return is;
}