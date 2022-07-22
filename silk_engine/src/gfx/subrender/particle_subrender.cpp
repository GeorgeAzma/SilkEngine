#include "particle_subrender.h"
#include "gfx/particle_system.h"
#include "gfx/renderer.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"

ParticleSubrender::ParticleSubrender(const PipelineStage& pipeline_stage)
    : pipeline()
{    
    using enum GpuType;
    pipeline.setShader(makeShared<Shader>("particle"))
        .setSamples(Graphics::swap_chain->getSamples())
        .setStage(pipeline_stage)
        .build();
}

void ParticleSubrender::render()
{
    ParticleSystem::render(pipeline);
}
