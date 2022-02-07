#pragma once

struct Subpass
{
	std::vector<VkAttachmentReference> input_attachment_references;
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> attachment_references;
	std::vector<VkAttachmentReference> color_attachment_references;
	VkAttachmentReference depth_stencil_attachment_reference;
	VkAttachmentReference resolve_attachment_reference;
	bool multisampled = false;
};

class RenderPass : NonCopyable
{
public:
	~RenderPass();

	RenderPass& addAttachment(VkFormat format, VkImageLayout image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	RenderPass& addSubpass();

	void build();

	void begin(VkFramebuffer framebuffer, VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void nextSubpass();
	void end();

	operator const VkRenderPass& () const { return render_pass; }

private:
	VkRenderPass render_pass;
	std::vector<Subpass> subpasses;
};