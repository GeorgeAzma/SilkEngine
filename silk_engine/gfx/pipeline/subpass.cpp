#include "subpass.h"

Subpass::Subpass(const Subpass& last_subpass)
{
    std::vector<VkAttachmentReference> input_attachment_references(last_subpass.attachments.size());
    for (size_t i = 0; i < last_subpass.attachments.size(); ++i)
        input_attachment_references[i] = last_subpass.attachments[i].reference;
    input_attachment_references = std::move(input_attachment_references);
}

void Subpass::addAttachment(const AttachmentProps& props)
{
    VkAttachmentDescription attachment_description{};
    attachment_description.format = VkFormat(props.format);
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
        multisampled = true;
    }

    Attachment attachment{};
    Attachment::Type type;
    attachment.description = attachment_description;

    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = attachments.size();
    if (Image::isDepthFormat(Image::Format(attachment_description.format)) ||
        Image::isStencilFormat(Image::Format(attachment_description.format)))
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_stencil_attachment_reference = attachment_reference;
        type = Attachment::Type::DEPTH_STENCIL;
    }
    else if (multisampled && props.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && (props.samples & VK_SAMPLE_COUNT_1_BIT))
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        resolve_attachment_reference = attachment_reference;
        type = Attachment::Type::RESOLVE;
    }
    else
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_references.push_back(attachment_reference);
        type = Attachment::Type::COLOR;
    }
    attachment.reference = attachment_reference;
    attachment.type = type;

    if (type == Attachment::Type::DEPTH_STENCIL)
        attachment.clear_value.depthStencil = props.clear_value ? props.clear_value->depthStencil : VkClearDepthStencilValue(1.0f, 0);
    else
        attachment.clear_value.color = props.clear_value ? props.clear_value->color : VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } };

    attachments.emplace_back(std::move(attachment));
}

void Subpass::build()
{
    description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    description.inputAttachmentCount = input_attachment_references.size();
    description.pInputAttachments = input_attachment_references.data();
    description.colorAttachmentCount = color_attachment_references.size();
    description.pColorAttachments = color_attachment_references.data();
    description.pDepthStencilAttachment = depth_stencil_attachment_reference ? &depth_stencil_attachment_reference.value() : nullptr;
    description.pResolveAttachments = resolve_attachment_reference ? &resolve_attachment_reference.value() : nullptr;
}