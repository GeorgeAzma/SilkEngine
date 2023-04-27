#include "particle_subrender.h"
#include "gfx/particle_system.h"
#include "gfx/renderer.h"
#include "gfx/devices/physical_device.h"
#include "gfx/debug_renderer.h"
#include "gfx/render_context.h"

ParticleSubrender::ParticleSubrender(const PipelineStage& pipeline_stage)
{
    shared<GraphicsPipeline> pipeline = makeShared<GraphicsPipeline>();
    pipeline->setShader(makeShared<Shader>("particle"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setStage(pipeline_stage)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::LESS)
        .build();
    material = makeShared<Material>(pipeline);
}

void ParticleSubrender::render()
{
    ParticleSystem::render(*material);
}
