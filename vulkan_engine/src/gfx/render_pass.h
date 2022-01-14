#pragma once

struct AttachmentProps
{
	VkFormat format;
	VkImageLayout image_layout;
	VkSampleCountFlagBits samples;
};

struct Subpass
{
	std::vector<VkAttachmentReference> input_attachments;
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> attachment_references;
	std::vector<VkAttachmentReference> color_attachment_references;
	VkAttachmentReference depth_attachment_reference;
	VkAttachmentReference resolve_attachment_reference;
};

class RenderPass : NonCopyable
{
public:
	~RenderPass();

	RenderPass& addAttachment(uint32_t attachment, VkFormat format, VkImageLayout image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	RenderPass& addResolveAttachment(VkFormat format);
	RenderPass& beginSubpass();

	void build();

	void begin(VkFramebuffer framebuffer);
	void end();


	operator const VkRenderPass& () const { return render_pass; }

private:
	
private:
	VkRenderPass render_pass;
	std::vector<Subpass> subpasses;
};