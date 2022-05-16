#pragma once

#include "gfx/images/image_format.h"

struct AttachmentProps
{
	ImageFormat format = ImageFormat::BGRA;
	VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

	std::optional<VkClearValue> clear_value = {};
	VkAttachmentLoadOp load_operation = VK_ATTACHMENT_LOAD_OP_CLEAR;
	VkAttachmentStoreOp store_operation = VK_ATTACHMENT_STORE_OP_STORE;
	VkAttachmentLoadOp stencil_load_operation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	VkAttachmentStoreOp stencil_store_operation = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

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

	RenderPass& addAttachment(const AttachmentProps& props);
	RenderPass& addSubpass();

	void build();

	void begin(VkFramebuffer framebuffer, VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void nextSubpass(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void end();

	operator const VkRenderPass& () const { return render_pass; }

private:
	VkRenderPass render_pass;
	std::vector<Subpass> subpasses;
};