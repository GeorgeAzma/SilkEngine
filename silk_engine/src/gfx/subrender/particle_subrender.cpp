#include "particle_subrender.h"
#include "gfx/particle_system.h"
#include "gfx/renderer.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"

ParticleSubrender::ParticleSubrender(const PipelineStage& pipeline_stage)
    : pipeline()
{    
    using enum DeviceType;
    pipeline.enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .setShader(makeShared<Shader>("particle"))
        .setVertexLayout({ { VEC2 }, { VEC2 }, { VEC4 }, {MAT4, 1}, {UINT, 1}, {VEC4, 1} })
        .setSamples(Graphics::swap_chain->getSamples())
        .setStage(pipeline_stage)
        .build();
}

void ParticleSubrender::render()
{
    ParticleSystem::render(pipeline);
}
