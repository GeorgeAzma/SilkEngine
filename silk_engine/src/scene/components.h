#pragma once

#include "model.h"
#include "instance.h"
#include "camera/camera.h"

struct TransformComponent
{
	glm::mat4 transform{1.0f};

	operator glm::mat4& () { return transform; }
};

struct ImageComponent
{
	ImageComponent(shared<Image2D> image) : images({ image }) {}
	ImageComponent(const std::vector<shared<Image2D>>& images) : images(images) {}

	std::vector<shared<Image2D>> images;

	ImageComponent& operator=(shared<Image2D> image) 
	{ 
		if (images.size()) 
			images[0] = image; 
		else 
			images = { image }; 
		return *this; 
	}

	operator const std::vector<shared<Image2D>>& () const { return images; }
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

	operator ScriptableEntity& () { return *instance; }
};

struct MeshComponent
{
	shared<Mesh> mesh;
	shared<RenderedInstance> instance = nullptr;
};

struct ModelComponent
{
	shared<Model> model;

	std::vector<shared<RenderedInstance>> instances{};

	operator Model& () { return *model; }
};

struct MaterialComponent
{
	shared<ShaderEffect> material;

	operator const ShaderEffect& () const { return *material; }
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
	std::string font = "Arial";

	operator std::string& () { return text; }
};

struct LightComponent
{
	LightComponent(const Light& light) : light(light) {}
	Light light = {};

	operator Light& () { return light; }

private:
	friend class Scene;
	Light* light_ptr = nullptr;
};