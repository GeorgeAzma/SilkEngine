#pragma once

#include "gfx/images/image.h"

struct AttachmentProps
{
	Image::Format format = Image::Format::BGRA;
	VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

	std::optional<VkClearValue> clear_value = {};
	VkAttachmentLoadOp load_operation = VK_ATTACHMENT_LOAD_OP_CLEAR;
	VkAttachmentStoreOp store_operation = VK_ATTACHMENT_STORE_OP_STORE;
	VkAttachmentLoadOp stencil_load_operation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	VkAttachmentStoreOp stencil_store_operation = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

class Subpass
{
public:
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

public:
	Subpass() = default;
	Subpass(const Subpass& last_subpass);

	void addAttachment(const AttachmentProps& props);
	void build();

	const VkSubpassDescription& getDescription() const { return description; }
	const std::vector<Attachment>& getAttachments() const { return attachments; }

private:
	VkSubpassDescription description{};
	std::vector<Attachment> attachments;
	std::vector<VkAttachmentReference> input_attachment_references;
	std::vector<VkAttachmentReference> color_attachment_references;
	std::optional<VkAttachmentReference> resolve_attachment_reference{};
	std::optional<VkAttachmentReference> depth_stencil_attachment_reference{};
	bool multisampled = false;
};