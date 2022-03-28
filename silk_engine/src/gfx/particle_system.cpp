#include "particle_system.h"
#include "scene/resources.h"
#include "graphics.h"
#include "window/swap_chain.h"
#include "utils/time.h"
#include "scene/scene_manager.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"
#include <glm/gtc/matrix_transform.hpp>

void ParticleSystem::init()
{
	auto mesh = Resources::getMesh("Quad");
    mesh->createVertexArray();
    vao = mesh->vertex_array;
	instance_vbo = makeShared<VertexBuffer>(nullptr, MAX_PARTICLES * sizeof(ParticleData));
	
	Resources::addGraphicsPipeline("Particle", []
                                   {
                                       shared<Shader> shader = makeShared<Shader>("particle");
                                       shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
                                       graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
                                           .enable(EnableTag::DEPTH_TEST)
                                           .enable(EnableTag::DEPTH_WRITE)
                                           .setShader(shader)
                                           .setVertexLayout({ { Type::VEC2 }, { Type::VEC2 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
                                           .setSampleCount(Graphics::swap_chain->getSampleCount())
                                           .setRenderPass(*Graphics::swap_chain->getRenderPass())
                                           .build();
                                       return graphics_pipeline;
                                   });
}

void ParticleSystem::emit(const ParticleProps& props)
{
    if (particles.size() >= MAX_PARTICLES) 
        return;

    particles.emplace_back
    (
        props.position,
        props.velocity,
        props.color_begin,
        props.color_end,
        RNG::Float() * glm::two_pi<float>(),
        RNG::Float() * glm::two_pi<float>(),
        props.size_begin,
        props.size_end,
        props.life_time,
        props.life_time,
        instance_images.add({ props.image.get() ? props.image : Resources::white_image })
    );
}

void ParticleSystem::update()
{
    for (auto& p : particles)
    {
        if (p.life_remaining <= 0)
        {
            std::swap(p, particles.back());
            particles.pop_back();
            continue;
        }
        p.life_remaining -= Time::dt;
        p.position += p.velocity * float(Time::dt);
    }

    Resources::pool.forEach(particles.size(), [](size_t i)
    {
        auto& p = particles[i];
        float life = p.life_remaining / p.life_time;
        glm::mat4& m = particle_data[i].model;
        const glm::mat4& v = SceneManager::getActive()->getMainCamera()->view;
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
    instance_vbo->setData(particle_data.data(), particle_data.size() * sizeof(ParticleData));
    
    auto pipeline = Resources::getGraphicsPipeline("Particle");
    pipeline->bind();
    vao->bind();
    instance_vbo->bind(1);
    auto images = pipeline->getShader()->get("images");
    instance_images.updateDescriptorSet(*pipeline->getShader()->getDescriptorSets().at(images.set), images.write_index);
    Graphics::getActiveCommandBuffer().drawIndexed(vao->getIndexBuffer()->getCount(), particle_data.size(), 0, 0, 0);
}