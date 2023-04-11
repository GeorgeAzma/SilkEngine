#include "mesh_subrender.h"
#include "gfx/devices/physical_device.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "scene/resources.h"
#include "gfx/debug_renderer.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/buffers/buffer.h"

MeshSubrender::MeshSubrender(const PipelineStage& pipeline_stage)
{
    using enum GpuType;
    shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("3D"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setStage(pipeline_stage)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::LESS)
        .build();
    Resources::add<GraphicsPipeline>("3D", graphics_pipeline);

    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("2D"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setStage(pipeline_stage)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::LESS_OR_EQUAL)
        .build();
    Resources::add<GraphicsPipeline>("2D", graphics_pipeline);

    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>("font"))
        .setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
        .setStage(pipeline_stage)
        .setDepthCompareOp(GraphicsPipeline::CompareOp::ALWAYS)
        .build();
    Resources::add<GraphicsPipeline>("Font", graphics_pipeline);
}

void MeshSubrender::render()
{
    //Draw instances
    size_t draw_index = 0;
    for (auto& instance_batch : DebugRenderer::instance_batches)
    {
        const auto& shader = instance_batch.instance->pipeline->getShader();
        if (auto global_uniform = shader->getLocation("GlobalUniform"))
        {
            auto descriptor_buffer_info = DebugRenderer::global_uniform_buffer->getDescriptorInfo();
            instance_batch.descriptor_sets.at(global_uniform->set)->update({ { global_uniform->binding, &descriptor_buffer_info } }); //TODO: Global data doesn't have to update for each batch
        }

        if (auto images = shader->getLocation("images"))
        {
            auto descriptor_image_infos = instance_batch.instance_images.getDescriptorImageInfos();
            instance_batch.descriptor_sets.at(images->set)->update({ { images->binding, descriptor_image_infos.data() } });
        }

        instance_batch.bind();

        RenderContext::submit([&](CommandBuffer& cb){ cb.drawIndexedIndirect(*DebugRenderer::indirect_buffer, draw_index * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand)); });
        ++draw_index;
    }
}