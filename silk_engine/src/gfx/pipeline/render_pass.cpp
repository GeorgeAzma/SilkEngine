#include "render_pass.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/enums.h"
#include "gfx/window/window.h"
#include "gfx/buffers/command_buffer.h"

RenderPass::~RenderPass()
{
    Graphics::logical_device->destroyRenderPass(render_pass);
}

RenderPass& RenderPass::addAttachment(const AttachmentProps& props)
{
    VkAttachmentDescription attachment_description{};
    attachment_description.format = ImageFormatEnum::toVulkanType(props.format);
    attachment_description.samples = props.samples;
    attachment_description.loadOp = props.load_operation;
    attachment_description.storeOp = props.store_operation;
    attachment_description.stencilLoadOp = props.stencil_load_operation;
    attachment_description.stencilStoreOp = props.stencil_store_operation;
    attachment_description.initialLayout = props.initial_layout;
    attachment_description.finalLayout = props.layout;

    if (props.samples != VK_SAMPLE_COUNT_1_BIT)
    {
        //Vulkan said that multisampled images should always have store op set to don't care (-\(^_^)/-)
        attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        subpasses.back().multisampled = true;
    }

    Attachment attachment{};
    Attachment::Type type;
    attachment.description = attachment_description;
    
    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = subpasses.back().attachments.size();
    if (ImageFormatEnum::hasDepth(ImageFormatEnum::fromVulkanType(attachment_description.format)) || 
        ImageFormatEnum::hasStencil(ImageFormatEnum::fromVulkanType(attachment_description.format)))
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        type = Attachment::Type::DEPTH_STENCIL;
    }
    else if (subpasses.back().multisampled && props.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        type = Attachment::Type::RESOLVE;
    }
    else
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        type = Attachment::Type::COLOR;
    }
    attachment.reference = attachment_reference;
    attachment.type = type;

    if (type == Attachment::Type::DEPTH_STENCIL)
        attachment.clear_value.depthStencil = props.clear_value ? props.clear_value->depthStencil : VkClearDepthStencilValue(1.0f, 0);
    else
        attachment.clear_value.color = props.clear_value ? props.clear_value->color : VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } };
    
    subpasses.back().attachments.emplace_back(std::move(attachment));
   
    return *this;
}

RenderPass& RenderPass::addSubpass()
{
    subpasses.emplace_back();

    if (subpasses.size() > 1)
    {
        auto& last_subpass = subpasses[subpasses.size() - 2];
        std::vector<VkAttachmentReference> input_attachment_references(last_subpass.attachments.size());
        for (size_t i = 0; i < last_subpass.attachments.size(); ++i)
            input_attachment_references[i] = last_subpass.attachments[i].reference;
        subpasses.back().input_attachment_references = std::move(input_attachment_references);
    }

    return *this;
}

void RenderPass::build()
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpass_descriptions(subpasses.size());
    std::vector<VkSubpassDependency> subpass_dependencies(subpasses.size());

    //We need these alive for till this function finishes
    std::vector<std::vector<VkAttachmentReference>> color_attachment_references(subpasses.size());
    std::vector<std::optional<VkAttachmentReference>> depth_stencil_attachment_references(subpasses.size());
    std::vector<std::optional<VkAttachmentReference>> resolve_attachment_references(subpasses.size());

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

        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = subpasses[i].input_attachment_references.size();
        subpass_description.pInputAttachments = subpasses[i].input_attachment_references.data();
        subpass_description.colorAttachmentCount = color_attachment_references[i].size();
        subpass_description.pColorAttachments = color_attachment_references[i].data();
        subpass_description.pDepthStencilAttachment = depth_stencil_attachment_references[i] ? &depth_stencil_attachment_references[i].value() : nullptr;
        subpass_description.pResolveAttachments = resolve_attachment_references[i] ? &resolve_attachment_references[i].value() : nullptr;
        subpass_descriptions[i] = std::move(subpass_description);

        //TODO:
        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = i ? (i - 1) : VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = i;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.srcAccessMask = VK_ACCESS_NONE;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpass_dependencies[i] = std::move(subpass_dependency);
    }
    
    VkRenderPassCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ci.attachmentCount = attachments.size();
    ci.pAttachments = attachments.data();
    ci.subpassCount = subpass_descriptions.size();
    ci.pSubpasses = subpass_descriptions.data();
    ci.dependencyCount = subpass_dependencies.size();
    ci.pDependencies = subpass_dependencies.data();
    render_pass = Graphics::logical_device->createRenderPass(ci);
}

void RenderPass::begin(VkFramebuffer framebuffer, VkSubpassContents subpass_contents)
{
    VkRenderPassBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.renderPass = render_pass;
    begin_info.framebuffer = framebuffer;

    begin_info.renderArea.offset = { 0, 0 };
    begin_info.renderArea.extent.width = Window::getWidth();
    begin_info.renderArea.extent.height = Window::getHeight();

    std::vector<VkClearValue> clear_values(subpasses.back().attachments.size());
    for (size_t i = 0; i < clear_values.size(); ++i)
        clear_values[i] = subpasses.back().attachments[i].clear_value;

    begin_info.clearValueCount = clear_values.size();
    begin_info.pClearValues = clear_values.data();

    Graphics::getActiveCommandBuffer().beginRenderPass(begin_info, subpass_contents);
}

void RenderPass::nextSubpass(VkSubpassContents subpass_contents)
{
    Graphics::getActiveCommandBuffer().nextSubpass(subpass_contents);
}

void RenderPass::end()
{
    Graphics::getActiveCommandBuffer().endRenderPass();
}