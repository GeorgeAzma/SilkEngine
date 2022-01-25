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
	Delta<glm::vec2> delta_mouse = glm::vec2(0);
};