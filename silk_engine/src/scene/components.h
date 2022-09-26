#pragma once

#include "model.h"
#include "entity.h"
#include "instance.h"
#include "light.h"
#include "gfx/ui/font.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "camera/camera.h"

struct TransformComponent
{
	mat4 transform{1.0f};

	operator mat4& () { return transform; }
};

struct ImageComponent
{
	ImageComponent(const shared<Image>& image) : images({ image }) {}
	ImageComponent(const std::vector<shared<Image>>& images) : images(images) {}

	std::vector<shared<Image>> images;

	ImageComponent& operator=(const shared<Image>& image) 
	{ 
		if (images.size()) 
			images[0] = image; 
		else 
			images = { image }; 
		return *this; 
	}

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

	operator ScriptableEntity& () { return *instance; }
};

struct MeshComponent
{
	shared<Mesh> mesh = nullptr;
	shared<RenderedInstance> instance = nullptr;
};

struct ModelComponent
{
	shared<Model> model = nullptr;

	std::vector<shared<RenderedInstance>> instances{};

	operator Model& () { return *model; }
};

struct MaterialComponent
{
	shared<GraphicsPipeline> material = nullptr;

	operator const GraphicsPipeline& () const { return *material; }
};

struct ColorComponent
{
	vec4 color = vec4(1);

	operator vec4& () { return color; }
};

struct TextComponent
{
	std::string text = "Text";

	size_t size = 32;
	shared<Font> font = nullptr;

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