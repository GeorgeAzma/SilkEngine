#include "framebuffer.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/gfx/window/swap_chain.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/pipeline/render_pass.h"

Framebuffer::Framebuffer(const SwapChain& swap_chain, const RenderPass& render_pass, uint32_t width, uint32_t height, bool imageless) :
    swap_chain(swap_chain),
    render_pass(render_pass),
    framebuffers(swap_chain.getImageCount()),
    attachments(swap_chain.getImageCount()),
    width(width),
    height(height)
{
    for (size_t attachment = 0; attachment < render_pass.getAttachmentDescriptions().size(); ++attachment)
    {
        const auto& attachment_desc = render_pass.getAttachmentDescriptions()[attachment];
        if (attachment_desc.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            for (size_t i = 0; i < swap_chain.getImages().size(); ++i)
                attachments[i].emplace_back(swap_chain.getImages()[i]);
        else
        { 
            // TODO: Remove hardcodes
            Image::Props image_props{};
            image_props.format = Format(attachment_desc.format);
            image_props.width = width;
            image_props.height = height;
            image_props.sampler_props.mipmap_mode = MipmapMode::NONE;
            image_props.samples = attachment_desc.samples;
            image_props.allocation_props.preferred_device = Allocation::Device::GPU;
            image_props.allocation_props.priority = 1.0f;
            image_props.usage = isDepthStencilFormat(Format(attachment_desc.format)) ? ImageUsage::DEPTH_STENCIL_ATTACHMENT : ImageUsage::COLOR_ATTACHMENT;
            // NOTE: Not sure if this is correct
            if (attachment_desc.loadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE && attachment_desc.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE)
            {
                if (attachment_desc.storeOp == VK_ATTACHMENT_STORE_OP_DONT_CARE && attachment_desc.stencilStoreOp == VK_ATTACHMENT_STORE_OP_DONT_CARE)
                    image_props.usage |= ImageUsage::TRANSIENT_ATTACHMENT;
                else
                    image_props.usage |= ImageUsage::SAMPLED;
            }
            if (render_pass.isInputAttachment(attachment))
                image_props.usage |= ImageUsage::INPUT_ATTACHMENT;
            
            shared<Image> image = makeShared<Image>(image_props);
            image->setLayout(attachment_desc.finalLayout);
            for (size_t i = 0; i < framebuffers.size(); ++i)
                attachments[i].emplace_back(image);
        }
    }

    for (size_t i = 0; i < framebuffers.size(); ++i)
    {
        std::vector<VkImageView> attachment_views(attachments[i].size());
        for (size_t j = 0; j < attachments[i].size(); ++j)
            attachment_views[j] = attachments[i][j]->getView();

        VkFramebufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ci.renderPass = render_pass;
        ci.attachmentCount = attachment_views.size();
        ci.pAttachments = attachment_views.data();
        ci.width = width;
        ci.height = height;
        ci.layers = 1;
        ci.flags = imageless * VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        framebuffers[i] = RenderContext::getLogicalDevice().createFramebuffer(ci);
    }
}

Framebuffer::~Framebuffer()
{
    for (size_t i = 0; i < framebuffers.size(); ++i)
        RenderContext::getLogicalDevice().destroyFramebuffer(framebuffers[i]);
}

const std::vector<shared<Image>>& Framebuffer::getAttachments() const
{
   return attachments[swap_chain.getImageIndex()];
}

Framebuffer::operator const VkFramebuffer& () const
{ 
    return framebuffers[swap_chain.getImageIndex()];
}
