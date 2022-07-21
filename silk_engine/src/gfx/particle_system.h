#pragma once

#include "utils/fixed_update.h"
#include "buffers/vertex_array.h"
#include "scene/instance_images.h"
#include "gfx/pipeline/graphics_pipeline.h"

struct ParticleProps
{
	glm::vec3 position = glm::vec3(0);
	glm::vec3 velocity = glm::vec3(0);
	glm::vec3 velocity_variation = glm::vec3(0);
	glm::vec3 acceleration = glm::vec3(0);
	glm::vec4 color_begin = glm::vec4(1);
	glm::vec4 color_end = glm::vec4(1);
	float size_begin = 1;
	float size_end = 1;
	shared<Image2D> image = nullptr;
	float life_time = 1.0f;
};

struct ParticleSpoutProps
{
	ParticleProps particle_properties;
	float duration = 0.0f;
	float particles_per_emission = 1;
	glm::vec3(*update_position)() = nullptr;
};

struct Particles
{
	static inline const ParticleProps flame{ {0, 0, 0}, { 0, 3, 0 }, { 1, 1, 1 }, { 0, 0, 0 }, {1, 0.75f, 0, 1}, {1, 0, 0, 0.0f}, .25f, .01f, nullptr, 1.0f };
};

class ParticleSystem
{
	struct Particle
	{
		glm::vec3 position;
		glm::vec3 velocity;
		glm::vec3 acceleration;
		glm::vec4 color_begin;
		glm::vec4 color_end;
		float rotation_begin;
		float rotation_end;
		float size_begin;
		float size_end;
		float life_time = 1.0f;
		float life_remaining = 0;
		uint32_t texture_index = 0;
	};

	struct ParticleSpout
	{
		ParticleProps particle_properties;
		float duration;
		float particles_per_emission;
		glm::vec3(*update_position)();
		float passed_duration;
	};

	struct ParticleData
	{
		glm::mat4 model;
		uint32_t texture_index;
		glm::vec4 color;
	};

public:
	static constexpr size_t MAX_PARTICLES = 65536;

public:
	static void init();

	static void addSpout(const ParticleSpoutProps& props);
	static void emit(const ParticleProps& props);
	
	static void update();
	static void render(GraphicsPipeline& graphics_pipeline);

	static void destroy();

private:
	static inline std::vector<Particle> particles;
	static inline std::vector<ParticleSpout> particle_spouts;
	static inline std::vector<ParticleData> particle_data;
	static inline shared<VertexArray> vao;
	static inline shared<VertexBuffer> instance_vbo;
	static inline shared<InstanceImages> instance_images;
};