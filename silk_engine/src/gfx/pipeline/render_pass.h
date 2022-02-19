#pragma once


struct Attachment
{
	enum class Type
	{
		COLOR, DEPTH_STENCIL, RESOLVE
	};
	VkAttachmentDescription description{};
	VkAttachmentReference reference{};
	VkClearValue clear_value{};
	Type type = Type::COLOR;
};

struct Subpass
{
	std::vector<VkAttachmentReference> input_attachment_references;
	std::vector<Attachment> attachments;
	bool multisampled = false;
};

class RenderPass : NonCopyable
{
public:
	~RenderPass();

	RenderPass& addAttachment(VkFormat format, VkImageLayout image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, std::optional<VkClearValue> clear_value = {}, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
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