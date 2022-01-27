#pragma once

#include "gfx/window/window.h"
#include "core/log.h"
#include "utils/math.h"
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

	operator uint32_t () const { return texture_index; }
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

	operator const ScriptableEntity* () const { return instance; }
};

struct MeshComponent
{
	shared<Mesh> mesh;

	operator const shared<Mesh>& () const { return mesh; }
};

struct RenderComponent
{
	RenderedInstance instance;

	operator const RenderedInstance& () const { return instance; }
};

struct ColorComponent
{
	glm::vec4 color = glm::vec4(1);

	operator const glm::vec4& () const { return color; }
};

struct TextComponent
{
	std::string test = "Text";

	size_t size = 32;
	std::string font = "arial.ttf";
};