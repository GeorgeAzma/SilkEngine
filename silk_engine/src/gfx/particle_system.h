#pragma once

#include "utils/fixed_update.h"
#include "buffers/vertex_array.h"
#include "scene/instance_images.h"

struct ParticleProps
{
	glm::vec3 position = glm::vec3(0);
	float life_time = 1.0f;
	glm::vec4 color_begin = glm::vec4(1);
	glm::vec4 color_end = glm::vec4(1);
	glm::vec3 velocity = glm::vec3(0);
	float size_begin = 1;
	glm::vec3 velocity_variation = glm::vec3(0);
	float size_end = 1;
	shared<Image2D> image = nullptr;
};

struct Particles
{
	static inline const ParticleProps flame{ {0, 0, 0}, 1.0f, {1, 0.75f, 0, 1}, {1, 0, 0, 0.25f}, glm::vec3(0, 3, 0), .25f, glm::vec3(1), .01f };
};


struct ParticleSpout
{
	ParticleSpout(const ParticleProps& props = Particles::flame, uint32_t emitions_per_second = 64, uint32_t particles_per_emition = 8)
		: props(props), emitions_per_second(FixedUpdate(emitions_per_second)), particles_per_emition(particles_per_emition)
	{
	}

	ParticleProps props;
	FixedUpdate emitions_per_second;
	uint32_t particles_per_emition;
};

class ParticleSystem
{
	struct Particle
	{
		glm::vec3 position;
		glm::vec3 velocity;
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

public:
	static constexpr size_t MAX_PARTICLES = 262144;

public:
	static void init();
	static void emit(const ParticleProps& props);
	static void update();

private:
	static inline std::vector<Particle> particles;
	struct ParticleData
	{
		glm::mat4 model;
		uint32_t texture_index;
		glm::vec4 color;
	};
	static inline std::vector<ParticleData> particle_data;
	static inline shared<VertexArray> vao;
	static inline shared<VertexBuffer> instance_vbo;
	static inline InstanceImages instance_images;
};