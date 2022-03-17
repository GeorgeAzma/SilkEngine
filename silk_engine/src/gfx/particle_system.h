#pragma once

#include "utils/fixed_update.h"
#include "buffers/vertex_array.h"
#include "scene/instance_images.h"

struct ParticleProps
{
	glm::vec3 position = glm::vec3(0);
	float lifeTime = 1.0f;
	glm::vec4 color_begin = glm::vec4(1);
	glm::vec4 color_end = glm::vec4(1);
	glm::vec3 velocity = glm::vec3(0);
	float size_begin = 1;
	glm::vec3 velocity_variation = glm::vec3(0);
	float size_end = 1;
	bool _2D = false;
};

struct Particles
{
	static constexpr ParticleProps flame{ {0, 0, 0}, 1.0f, {1, 0.75f, 0, 1}, {1, 0, 0, 0.25f}, glm::vec3(0, 3, 0), .25f, glm::vec3(1), .01f, false };
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
		glm::vec3 rotation_begin;
		glm::vec3 rotation_end;
		float size_begin, size_end;
		float life_time = 1.0f;
		float life_remaining = 0;
		uint32_t texture_index = 0;
	};

public:
	static void init();
	static void emit(const ParticleProps& props);
	static void update();

	static void addSpout(const std::string& spout_name, const ParticleSpout& spout);
	static void removeSpout(const std::string& spout_name);

private:
	static inline std::vector<Particle> particles;
	struct ParticleData
	{
		glm::mat4 model;
		glm::vec4 color;
		uint32_t texture_index;
	};
	static inline std::vector<ParticleData> particle_data;
	struct ParticleData2D
	{
		glm::mat3 model;
		glm::vec4 color;
		uint32_t texture_index;
	};
	static inline std::vector<ParticleData2D> particle_data2D;
	static inline shared<VertexArray> vao;
	static inline shared<VertexArray> vao2D;
	static inline InstanceImages instance_images;
};