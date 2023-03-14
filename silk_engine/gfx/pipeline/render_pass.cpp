#include "render_pass.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/window/window.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/buffers/framebuffer.h"

RenderPass::~RenderPass()
{
    Graphics::logical_device->destroyRenderPass(render_pass);
}

RenderPass& RenderPass::addAttachment(const AttachmentProps& props)
{
    subpasses.back().addAttachment(props);
    return *this;
}

RenderPass& RenderPass::addSubpass()
{
    if (subpasses.size() >= 1)
        subpasses.emplace_back(subpasses.back());
    else
        subpasses.emplace_back();

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
        for (const auto& attachment : subpasses[i].getAttachments())
            attachments.push_back(attachment.description);
        subpasses[i].build();
        subpass_descriptions[i] = subpasses[i].getDescription();

        //TODO:
        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = i ? (i - 1) : VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = i;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.srcAccessMask = VK_ACCESS_NONE;
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
    
void RenderPass::begin(const Framebuffer& framebuffer, VkSubpassContents subpass_contents)
{
    Graphics::submit(
        [&](CommandBuffer& cb)
        {
            VkRenderPassBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            begin_info.renderPass = render_pass;
            begin_info.framebuffer = framebuffer;

            begin_info.renderArea.offset = { 0, 0 };
            begin_info.renderArea.extent.width = framebuffer.getWidth();
            begin_info.renderArea.extent.height = framebuffer.getHeight();

            std::vector<VkClearValue> clear_values;
            for (const auto& subpass : subpasses)
                for (const auto& attachment : subpass.getAttachments())
                    clear_values.emplace_back(attachment.clear_value);

            begin_info.clearValueCount = clear_values.size();
            begin_info.pClearValues = clear_values.data();

            cb.beginRenderPass(begin_info, subpass_contents);
        });
}

void RenderPass::nextSubpass(VkSubpassContents subpass_contents)
{
    Graphics::submit(
        [&](CommandBuffer& cb)
        {
            if (cb.getActive().subpass == (subpasses.size() - 1))
            {
                SK_ERROR("nextSubpass was called when there was no next subpass");
                return;
            }
            cb.nextSubpass(subpass_contents);
        });
}

void RenderPass::end()
{
    Graphics::submit([&](CommandBuffer& cb) { cb.endRenderPass(); });
}