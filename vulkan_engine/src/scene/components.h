#pragma once

#include "gfx/window/window.h"
#include "core/log.h"
#include "utils/math.h"
#include "scene/render_object.h"

#undef near
#undef far

struct TransformComponent
{
	glm::mat4 transform{1.0f};

	operator glm::mat4& () { return transform; }
};

enum class CameraType
{
	PERSPECTIVE,
	ORTHOGRAPHIC
};

struct CameraComponent
{
public:
	CameraComponent() { onViewportResize(); }

	operator const glm::mat4& () { return projection_view; }

	void onViewportResize()
	{
		switch (type)
		{
		case CameraType::PERSPECTIVE:
			projection = glm::perspective(glm::radians(fov), Window::getAspectRatio(), near, far);
			break;
		case CameraType::ORTHOGRAPHIC:
			projection = glm::ortho(0.0f, (float)Window::getWidth(), 0.0f, (float)Window::getHeight(), 0.0f, 1.0f);
			break;
		}
		projection_view = projection * view;
	}

	void setFOV(float fov)
	{
		this->fov = fov;
		switch(type)
		{
		case CameraType::PERSPECTIVE:
			projection = glm::perspective(glm::radians(fov), Window::getAspectRatio(), near, far);
			break;
		case CameraType::ORTHOGRAPHIC:
			projection = glm::ortho(0.0f, (float)Window::getWidth(), 0.0f, (float)Window::getHeight(), 0.0f, 1.0f);
			break;
		}
		projection_view = projection * view;
	}

	void setNear(float near)
	{
		//VE_ASSERT(type == CameraType::PERSPECTIVE, "Only perspective cameras have near variable");
		this->near = near;
		projection = glm::perspective(glm::radians(fov), (float)Window::getAspectRatio(), near, far);
		projection_view = projection * view;
	}

	void setFar(float far)
	{
		//VE_ASSERT(type == CameraType::PERSPECTIVE, "Only perspective cameras have near variable");
		this->far = far;
		projection = glm::perspective(glm::radians(fov), (float)Window::getAspectRatio(), near, far);
		projection_view = projection * view;
	}

	glm::mat4 projection_view = glm::mat4(1);
	glm::mat4 projection = glm::mat4(1);
	glm::mat4 view = glm::mat4(1);

	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 direction = glm::vec3(0);

private:
	float fov = 80.0f;
	float near = 0.01f;
	float far = 1000.0f;

	CameraType type = CameraType::PERSPECTIVE;
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