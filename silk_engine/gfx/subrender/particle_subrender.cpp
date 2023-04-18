#include "particle_subrender.h"
#include "gfx/particle_system.h"
#include "gfx/renderer.h"
#include "gfx/devices/physical_device.h"

ParticleSubrender::ParticleSubrender(const PipelineStage& pipeline_stage)
    : pipeline()
{    
    pipeline.setShader(makeShared<Shader>("particle"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setStage(pipeline_stage)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::LESS)
        .build();
}

void ParticleSubrender::render()
{
    ParticleSystem::render(pipeline);
}
