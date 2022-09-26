#include "my_scene.h"
#include "gfx/window/window.h"
#include "neat/genome.h"

/*
Car
Starts
Forwards
Uses output to go
Crashes
Everyone else crashes
Fitness calculates
Parents selects
Babies born via crossover
Babies replace parents
Speciated
*/

void MyScene::onStart()
{
    Window::setSize({ 1280, 720 });
    neat = makeUnique<Neat>(3, 2);
    
    genome1 = makeShared<Genome>(*neat);
    genome1->addConnection(Gene{ neat->getBiasIndex(), neat->getOutputIndex() });
    genome1->addConnection(Gene{ neat->getBiasIndex(), neat->getOutputIndex() + 1 });
    
    genome2 = makeShared<Genome>(*neat);
    genome2->addConnection(Gene{ neat->getBiasIndex(), neat->getOutputIndex() });
    genome2->addConnection(Gene{ neat->getBiasIndex(), neat->getOutputIndex() + 1 });

    neat->addGenome(genome1);
    neat->addGenome(genome2);
    Genome genome = genome1->crossover(*genome2);
    genome1 = makeShared<Genome>(genome);

    genome.forward({ 2.0f, 3.5f, 1.0f });
    SK_TRACE("{}, {}", genome.getOutput()[0], genome.getOutput()[1]);
}

void MyScene::onUpdate()
{
    //genome1->forward({ 2.0f, 3.5f, 1.0f });
    //SK_TRACE("{}, {}", genome1->getOutput()[0], genome1->getOutput()[1]);
}

void MyScene::onStop()
{
}
