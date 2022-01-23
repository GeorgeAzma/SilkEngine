#pragma once

#include "gfx/window/window.h"
#include "core/log.h"
#include "utils/math.h"
#include "scene/render_object.h"
#include "scene/camera.h"

struct TransformComponent
{
	glm::mat4 transform{1.0f};

	operator glm::mat4& () { return transform; }
};

struct CameraComponent
{
	Camera camera;

	operator Camera& () { return camera; }
};

struct ScriptComponent
{
	ScriptableEntity* instance = nullptr;

	ScriptableEntity*(*instantiate_script)();
	void(*destroy_script)(ScriptComponent*);

	template<typename T>
	void bind()
	{
		instantiate_script = [] 
		{ 
			return static_cast<ScriptableEntity*>(new T()); 
		};
		destroy_script = [](ScriptComponent* script_component) 
		{ 
			delete (T*)script_component->instance;
			script_component->instance = nullptr;
		};
	}
};

struct MeshComponent
{
	shared<Mesh> mesh;
};

struct RenderComponent
{
	RenderObject render_object;
};