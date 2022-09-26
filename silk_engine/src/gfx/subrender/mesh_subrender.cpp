#include "mesh_subrender.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "scene/resources.h"
#include "gfx/renderer.h"
#include "gfx/buffers/command_buffer.h"
#include "scene/instance.h"

MeshSubrender::MeshSubrender(const PipelineStage& pipeline_stage)
{
    using enum GpuType;
    shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("3D"))
        .setSamples(Graphics::swap_chain->getSamples())
        .setStage(pipeline_stage)
        .build();
    Resources::add<GraphicsPipeline>("3D", graphics_pipeline);

    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("2D"))
        .setSamples(Graphics::swap_chain->getSamples())
        .setStage(pipeline_stage)
        .build();
    Resources::add<GraphicsPipeline>("2D", graphics_pipeline);

    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("font"))
        .setSamples(Graphics::swap_chain->getSamples())
        .setStage(pipeline_stage)
        .build();
    Resources::add<GraphicsPipeline>("Font", graphics_pipeline);
}

void MeshSubrender::render()
{
    //Draw instances
    size_t draw_index = 0;
    for (auto& instance_batch : Renderer::getInstanceBatches())
    {
        const auto& shader = instance_batch.instance->pipeline->getShader();
        if (auto global_uniform = shader->getIfExists("GlobalUniform"))
            instance_batch.descriptor_sets[global_uniform->set].setBufferInfo(global_uniform->binding, { *Renderer::getGlobalUniformBuffer() }); //TODO: Global data doesn't have to update for each batch

        if (auto images = shader->getIfExists("images"))
            instance_batch.descriptor_sets[images->set].setImageInfo(images->binding, instance_batch.instance_images.getDescriptorImageInfos());

        instance_batch.bind();

        Graphics::submit([&](CommandBuffer& cb){ cb.drawIndexedIndirect(*Renderer::getIndirectBuffer(), draw_index * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand)); });
        ++draw_index;
    }
}