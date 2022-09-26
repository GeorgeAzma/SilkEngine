#pragma once

#include "scene/entity.h"
#include "utils/delta.h"

class CameraController : public ScriptableEntity
{
public:
	void onCreate();
	void onUpdate();
	void onDestroy();

private:
	Delta<vec2> delta_mouse = vec2(0);
};