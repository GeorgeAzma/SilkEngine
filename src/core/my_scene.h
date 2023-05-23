#pragma once

#include "silk_engine/scene/scene.h"
#include "silk_engine/gfx/images/image.h"

class World;
class RenderGraph;

class MyScene : public Scene
{
public:
	void onStart() override;
	void onUpdate() override;
	void onStop() override;

private:
	shared<World> world = nullptr;
	shared<RenderGraph> render_graph = nullptr;
};