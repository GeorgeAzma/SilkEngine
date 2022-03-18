#pragma once

#include "scene/instance.h"
#include "scene/camera/camera.h"
#include "buffers/uniform_buffer.h"
#include "buffers/indirect_buffer.h"
#include "scene/light.h"

class Renderer
{
public:
	static constexpr size_t MAX_LIGHTS = 64;

public:
	struct GlobalUniformData
	{
		glm::mat4 projection_view;
		glm::mat4 projection_view2D;
		glm::vec3 camera_position;
		float time;
		glm::vec3 camera_direction;
		float delta_time;
		glm::uvec2 resolution;
		uint32_t frame;
		uint32_t padding;
		std::array<Light, MAX_LIGHTS> lights;
	};

public:
	static void init();
	static void update(Camera* camera = nullptr);
	static void cleanup();

	static Light* addLight(const Light& light);
	static void createMeshInstance(const shared<RenderedInstance>& instance, const InstanceData& instance_data);
	static void addInstanceBatch(const shared<RenderedInstance>& instance, const InstanceData& instance_data);
	static void destroyMeshInstance(const RenderedInstance& instance);

public:
	static inline std::vector<InstanceBatch> instance_batches;
	static inline unique<IndirectBuffer> indirect_buffer;
	static inline unique<UniformBuffer> global_uniform_buffer;
	static inline std::array<Light, MAX_LIGHTS> lights;
};