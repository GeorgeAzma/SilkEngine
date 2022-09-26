#pragma once

#include "scene/scene.h"

class MyScene : public Scene
{
public:
	void onStart() override;
	void onUpdate() override;
	void onStop() override;

private:
};