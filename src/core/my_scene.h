#pragma once

#include "silk_engine/scene/scene.h"
#include "silk_engine/gfx/images/image.h"

class MyScene : public Scene
{
public:
	void onStart() override;
	void onUpdate() override;
	void onStop() override;

private:
	shared<Entity> camera;
	shared<Image> image;
	std::vector<u8vec4> pixels;
};