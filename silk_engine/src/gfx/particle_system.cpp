#include "particle_system.h"
#include "scene/resources.h"
#include "graphics.h"
#include "window/swap_chain.h"
#include "utils/time.h"
#include "scene/scene_manager.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"
#include "utils/thread_pool.h"
#include "renderer.h"
#include "gfx/buffers/uniform_buffer.h"

void ParticleSystem::init()
{
    vao = Resources::get<Mesh>("Quad")->getVertexArray();
	instance_vbo = makeShared<VertexBuffer>(nullptr, MAX_PARTICLES * sizeof(ParticleData));
    instance_images = makeShared<InstanceImages>();
}

void ParticleSystem::addSpout(const ParticleSpoutProps& props)
{
    ParticleSpout spout;
    spout.duration = props.duration;
    spout.particles_per_emission = props.particles_per_emission;
    spout.particle_properties = props.particle_properties;
    spout.passed_duration = 0.0f;
    spout.update_position = props.update_position;
    particle_spouts.emplace_back(std::move(spout));
}

void ParticleSystem::emit(const ParticleProps& props)
{
    if (particles.size() >= MAX_PARTICLES) 
        return;

    particles.emplace_back
    (
        props.position,
        props.velocity + props.velocity_variation * (float)RNG::Float() * RNG::Vec3(),
        props.acceleration,
        props.color_begin,
        props.color_end,
        RNG::Float() * glm::two_pi<float>(),
        RNG::Float() * glm::two_pi<float>(),
        props.size_begin,
        props.size_end,
        props.life_time,
        props.life_time,
        instance_images->add({ props.image.get() ? props.image : Resources::white_image })
    );
}

void ParticleSystem::update()
{
    float dt = float(Time::dt);
    for (size_t i = 0; i < particle_spouts.size(); ++i)
    {
        auto& ps = particle_spouts[i];
        if (ps.passed_duration > ps.duration)
        {
            std::swap(ps, particle_spouts.back());
            particle_spouts.pop_back();
            continue;
        }
        if (ps.update_position)
            ps.particle_properties.position = ps.update_position();
        for (size_t i = 0; i < ps.particles_per_emission; ++i)
            emit(ps.particle_properties);
        ps.passed_duration += dt;
    }

    for (size_t i = 0; i < particles.size(); ++i)
    {
        auto& p = particles[i];
        if (p.life_remaining <= 0)
        {
            std::swap(p, particles.back());
            particles.pop_back();
            continue;
        }
        p.life_remaining -= dt;
        p.velocity += p.acceleration * dt;
        p.position += p.velocity * dt;
    }

    particle_data.resize(particles.size());
    auto camera = SceneManager::getActive()->getMainCamera();
    if (camera)
    {
        Resources::pool.forEach(particles.size(), [&](size_t i)
            {
                auto& p = particles[i];
                float life = 1.0f - p.life_remaining / p.life_time;
                glm::mat4& m = particle_data[i].model;
                const glm::mat4& v = camera->view;
                m = glm::translate(glm::mat4(1), p.position);
                m[0][0] = v[0][0];
                m[0][1] = v[1][0];
                m[0][2] = v[2][0];
                m[1][0] = v[0][1];
                m[1][1] = v[1][1];
                m[1][2] = v[2][1];
                m[2][0] = v[0][2];
                m[2][1] = v[1][2];
                m[2][2] = v[2][2];
                m = glm::rotate(glm::scale(m, glm::vec3(std::lerp(p.size_begin, p.size_end, life))), std::lerp(p.rotation_begin, p.rotation_end, life), { 0, 0, 1 });
                particle_data[i].color = math::lerp(p.color_begin, p.color_end, life);
                particle_data[i].texture_index = p.texture_index;
            });
    }
    instance_vbo->setData(particle_data.data(), particle_data.size() * sizeof(ParticleData));
}

void ParticleSystem::render(GraphicsPipeline& graphics_pipeline)
{
    if (particle_data.size())
    {
        graphics_pipeline.bind();
        vao->bind();
        instance_vbo->bind(1);
        graphics_pipeline.getShader()->setIfExists("GlobalUniform", { *Renderer::getGlobalUniformBuffer() }); //TODO: Unsafe, change
        graphics_pipeline.getShader()->setIfExists("images", instance_images->getDescriptorImageInfos());
        graphics_pipeline.getShader()->bindDescriptorSets();
        Graphics::getActiveCommandBuffer().drawIndexed(vao->getIndexBuffer()->getCount(), particle_data.size(), 0, 0, 0);
    }
}

void ParticleSystem::destroy()
{
    vao = nullptr;
    instance_vbo = nullptr;
    instance_images = nullptr;
}