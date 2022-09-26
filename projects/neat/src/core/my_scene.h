#pragma once

#include "scene/scene.h"
#include "neat/neat.h"

class MyScene : public Scene
{
public:
	void onStart() override;
	void onUpdate() override;
	void onStop() override;

private:
	unique<Neat> neat;
	shared<Genome> genome1;
	shared<Genome> genome2;
};