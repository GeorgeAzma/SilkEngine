#include "render_pass.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/enums.h"
#include "gfx/window/swap_chain.h"

RenderPass::~RenderPass()
{
    Graphics::logical_device->destroyRenderPass(render_pass);
}

RenderPass& RenderPass::addAttachment(const AttachmentProps& props)
{
    vk::AttachmentDescription attachment_description{};
    attachment_description.format = props.format;
    attachment_description.samples = props.samples;
    attachment_description.loadOp = props.load_operation;
    attachment_description.storeOp = props.store_operation;
    attachment_description.stencilLoadOp = props.stencil_load_operation;
    attachment_description.stencilStoreOp = props.stencil_store_operation;
    attachment_description.initialLayout = props.initial_layout;
    attachment_description.finalLayout = props.layout;

    if (props.samples != vk::SampleCountFlagBits::e1)
        subpasses.back().multisampled = true;

    Attachment attachment{};
    Attachment::Type type;
    attachment.description = attachment_description;
    
    vk::AttachmentReference attachment_reference{};
    attachment_reference.attachment = subpasses.back().attachments.size();
    if (Image::hasDepth(attachment_description.format) || Image::hasStencil(attachment_description.format))
    {
        attachment_reference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        type = Attachment::Type::DEPTH_STENCIL;
    }
    else if (subpasses.back().multisampled && props.layout == vk::ImageLayout::ePresentSrcKHR)
    {
        attachment_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;
        type = Attachment::Type::RESOLVE;
    }
    else
    {
        attachment_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;
        type = Attachment::Type::COLOR;
    }
    attachment.reference = attachment_reference;
    attachment.type = type;

    if (type == Attachment::Type::DEPTH_STENCIL)
        attachment.clear_value.depthStencil = props.clear_value ? props.clear_value->depthStencil : vk::ClearDepthStencilValue(1.0f, 0);
    else
        attachment.clear_value.color = props.clear_value ? props.clear_value->color : vk::ClearColorValue({ 0.0f, 0.0f, 0.0f, 1.0f });
    
    subpasses.back().attachments.emplace_back(std::move(attachment));
   
    return *this;
}

RenderPass& RenderPass::addSubpass()
{
    subpasses.emplace_back();

    if (subpasses.size() > 1)
    {
        auto& last_subpass = subpasses[subpasses.size() - 2];
        std::vector<vk::AttachmentReference> input_attachment_references(last_subpass.attachments.size());
        for (size_t i = 0; i < last_subpass.attachments.size(); ++i)
            input_attachment_references[i] = last_subpass.attachments[i].reference;
        subpasses.back().input_attachment_references = std::move(input_attachment_references);
    }

    return *this;
}

void RenderPass::build()
{
    std::vector<vk::AttachmentDescription> attachments;
    std::vector<vk::SubpassDescription> subpass_descriptions(subpasses.size());
    std::vector<vk::SubpassDependency> subpass_dependencies(subpasses.size());

    //We need these alive for till this function finishes
    std::vector<std::vector<vk::AttachmentReference>> color_attachment_references(subpasses.size());
    std::vector<vk::AttachmentReference> depth_stencil_attachment_references(subpasses.size());
    std::vector<vk::AttachmentReference> resolve_attachment_references(subpasses.size());

    for (size_t i = 0; i < subpasses.size(); ++i)
    {
        //Sort attachments in 3 categories
        for (auto& attachment : subpasses[i].attachments)
        {
            switch (attachment.type)
            {
            case Attachment::Type::COLOR:
                color_attachment_references[i].emplace_back(attachment.reference);
                break;
            case Attachment::Type::DEPTH_STENCIL:
                depth_stencil_attachment_references[i] = attachment.reference;
                break;
            case Attachment::Type::RESOLVE:
                resolve_attachment_references[i] = attachment.reference;
                break;
            }
            attachments.emplace_back(attachment.description);
        }

        vk::SubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass_description.inputAttachmentCount = subpasses[i].input_attachment_references.size();
        subpass_description.pInputAttachments = subpasses[i].input_attachment_references.data();
        subpass_description.colorAttachmentCount = color_attachment_references[i].size();
        subpass_description.pColorAttachments = color_attachment_references[i].data();
        subpass_description.pDepthStencilAttachment = &depth_stencil_attachment_references[i];
        subpass_description.pResolveAttachments = &resolve_attachment_references[i];
        subpass_descriptions[i] = std::move(subpass_description);

        //TODO:
        vk::SubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = i ? (i - 1) : VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = i;
        subpass_dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        subpass_dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        subpass_dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        subpass_dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        subpass_dependencies[i] = std::move(subpass_dependency);
    }
    
    vk::RenderPassCreateInfo ci{};
    ci.attachmentCount = attachments.size();
    ci.pAttachments = attachments.data();
    ci.subpassCount = subpass_descriptions.size();
    ci.pSubpasses = subpass_descriptions.data();
    ci.dependencyCount = subpass_dependencies.size();
    ci.pDependencies = subpass_dependencies.data();
    render_pass = Graphics::logical_device->createRenderPass(ci);
}

void RenderPass::begin(vk::Framebuffer framebuffer, vk::SubpassContents subpass_contents)
{
    if (Graphics::active.render_pass == render_pass)
        return;

    vk::RenderPassBeginInfo begin_info{};
    begin_info.renderPass = render_pass;
    begin_info.framebuffer = framebuffer;

    begin_info.renderArea.offset = vk::Offset2D(0, 0);
    begin_info.renderArea.extent = Graphics::swap_chain->getExtent();

    std::vector<vk::ClearValue> clear_values(subpasses.back().attachments.size());
    for (size_t i = 0; i < clear_values.size(); ++i)
        clear_values[i] = subpasses.back().attachments[i].clear_value;

    begin_info.clearValueCount = clear_values.size();
    begin_info.pClearValues = clear_values.data();

    Graphics::active.command_buffer.beginRenderPass(begin_info, subpass_contents);
    Graphics::active.render_pass = render_pass;
}

void RenderPass::nextSubpass(vk::SubpassContents subpass_contents)
{
    Graphics::active.command_buffer.nextSubpass(subpass_contents);
}

void RenderPass::end()
{
    if (Graphics::active.render_pass != render_pass)
        return;

    Graphics::active.command_buffer.endRenderPass();

    Graphics::active.render_pass = VK_NULL_HANDLE;
    if (Graphics::active.bind_point == vk::PipelineBindPoint::eGraphics)
    {
        Graphics::active.pipeline = VK_NULL_HANDLE;
        Graphics::active.bind_point = vk::PipelineBindPoint(VK_PIPELINE_BIND_POINT_MAX_ENUM);
    }
}