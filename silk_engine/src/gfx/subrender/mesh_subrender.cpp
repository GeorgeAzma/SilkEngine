#include "mesh_subrender.h"

#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "scene/resources.h"
#include "gfx/renderer.h"

MeshSubrender::MeshSubrender(const PipelineStage& pipeline_stage)
{
    using enum DeviceType;
    shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .setShader(Resources::getShader("3D"))
        .setVertexLayout({ { VEC3 }, { VEC2 }, { VEC3 }, { VEC4 }, { MAT4, 1 }, { UINT, 1 }, { VEC4, 1 } })
        .setSamples(Graphics::swap_chain->getSamples())
        .setRenderPass(*Renderer::getRenderPass(pipeline_stage))
        .build();
    Resources::addGraphicsPipeline("Lit 3D", graphics_pipeline);

    VkBool32 lit = false;
    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .setShader(Resources::getShader("3D"), { { "lit", &lit, sizeof(VkBool32) } })
        .setVertexLayout({ { VEC3 }, { VEC2 }, { VEC3 }, { VEC4 }, { MAT4, 1 }, { UINT, 1 }, { VEC4, 1 } })
        .setSamples(Graphics::swap_chain->getSamples())
        .setRenderPass(*Renderer::getRenderPass(pipeline_stage))
        .build();
    Resources::addGraphicsPipeline("3D", graphics_pipeline);

    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .setDepthCompareOp(VK_COMPARE_OP_ALWAYS)
        .setShader(Resources::getShader("2D"))
        .setVertexLayout({ { VEC2 }, { VEC2 }, { VEC4 }, { MAT4, 1 }, { UINT, 1 }, { VEC4, 1 } })
        .setSamples(Graphics::swap_chain->getSamples())
        .setRenderPass(*Renderer::getRenderPass(pipeline_stage))
        .build();
    Resources::addGraphicsPipeline("2D", graphics_pipeline);

    graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .setDepthCompareOp(VK_COMPARE_OP_ALWAYS)
        .setShader(Resources::getShader("Font"))
        .setVertexLayout({ { VEC2 }, { VEC2 }, { VEC4 }, { MAT4, 1 }, { UINT, 1 }, { VEC4, 1 } })
        .setSamples(Graphics::swap_chain->getSamples())
        .setRenderPass(*Renderer::getRenderPass(pipeline_stage))
        .build();
    Resources::addGraphicsPipeline("Font", graphics_pipeline);
}

void MeshSubrender::render()
{
    //Draw instances
    size_t draw_index = 0;
    for (auto& instance_batch : Renderer::getInstanceBatches())
    {
        const auto& shader = instance_batch.instance->material->getShader();
        if (auto global_uniform = shader->getIfExists("GlobalUniform"))
            instance_batch.descriptor_sets[global_uniform->set].setBufferInfo(global_uniform->binding, { *Renderer::getGlobalUniformBuffer() }); //TODO: Global data doesn't have to update for each batch

        if (auto images = shader->getIfExists("images"))
            instance_batch.descriptor_sets[images->set].setImageInfo(images->binding, instance_batch.instance_images.getDescriptorImageInfos());

        instance_batch.bind();
        Graphics::getActiveCommandBuffer().drawIndexedIndirect(*Renderer::getIndirectBuffer(), draw_index * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
        ++draw_index;
    }
}