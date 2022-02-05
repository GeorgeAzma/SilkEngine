#pragma once

#include "gfx/window/window.h"
#include "core/log.h"
#include "utils/math.h"
#include "scene/model.h"
#include "scene/instance.h"
#include "scene/camera/camera.h"

struct TransformComponent
{
	glm::mat4 transform{1.0f};

	operator glm::mat4& () { return transform; }
};

struct SpriteComponent
{
	uint32_t texture_index = 0;

	operator uint32_t& () { return texture_index; }
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

	operator ScriptableEntity* () { return instance; }
};

struct MeshComponent
{
	shared<RenderedInstance> instance;

	operator RenderedInstance& () { return *instance; }
};

struct ModelComponent
{
	shared<Model> model;

	std::vector<InstanceData*> instance_data;

	operator Model& () { return *model; }
};

struct ColorComponent
{
	glm::vec4 color = glm::vec4(1);

	operator glm::vec4& () { return color; }
};

struct TextComponent
{
	std::string text = "Text";

	size_t size = 32;
	std::string font = "arial.ttf";

	operator std::string& () { return text; }
};

struct LightComponent
{
	Light light;

	operator Light& () { return light; };
};