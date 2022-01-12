#pragma once

#include "entity.h"

class CameraController : public ScriptableEntity
{
public:
	void onCreate();
	void onUpdate();
	void onDestroy();
};