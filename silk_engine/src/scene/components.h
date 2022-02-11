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

struct ImageComponent
{
	ImageComponent(shared<Image> image) : images({ image }) {}
	ImageComponent(const std::vector<shared<Image>>& images) : images(images) {}

	std::vector<shared<Image>> images;

	operator const std::vector<shared<Image>>& () const { return images; }
};

struct BufferComponent
{
	std::vector<shared<Buffer>> buffers;

	operator const std::vector<shared<Buffer>>& () const { return buffers; }
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
	shared<Mesh> mesh;
	shared<RenderedInstance> instance = nullptr;

	operator Mesh& () { return *mesh; }
};

struct ModelComponent
{
	shared<Model> model;

	std::vector<shared<RenderedInstance>> instances;

	operator Model& () { return *model; }
};

struct MaterialComponent
{
	shared<Material> material;

	operator const Material& () const { return *material; }
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
	Light light = {};
	Light* light_ptr = nullptr; //Never touch this, it's used for scene's internal code

	operator Light& () { return light; };
};