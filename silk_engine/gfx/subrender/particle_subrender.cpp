#include "particle_subrender.h"
#include "gfx/particle_system.h"
#include "gfx/renderer.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"

ParticleSubrender::ParticleSubrender(const PipelineStage& pipeline_stage)
    : pipeline()
{    
    using enum GpuType;
    pipeline.setShader(makeShared<Shader>("particle"))
        .setSamples(Window::getActive().getSwapChain().getSamples())
        .setStage(pipeline_stage)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::LESS)
        .build();
}

void ParticleSubrender::render()
{
    ParticleSystem::render(pipeline);
}
