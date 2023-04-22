#pragma once

#include "gfx/images/image.h"

class Framebuffer;
class SwapChain;

struct AttachmentProps
{
	Image::Format format = Image::Format::BGRA;
	VkImageLayout final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentLoadOp load_operation = VK_ATTACHMENT_LOAD_OP_CLEAR;
	std::optional<VkClearValue> clear_value = std::nullopt;
	VkAttachmentStoreOp store_operation = VK_ATTACHMENT_STORE_OP_STORE;
	VkAttachmentLoadOp stencil_load_operation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	VkAttachmentStoreOp stencil_store_operation = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct RenderTargetInfo
{
	Image::Format format = Image::Format::BGRA;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	bool presented = false;
};

struct SubpassProps
{
	std::vector<AttachmentProps> outputs;
	std::vector<uint32_t> inputs;
};

class RenderPass : NonCopyable
{
public:
	RenderPass(const std::vector<SubpassProps>& subpass_props);
	~RenderPass();

	void begin(const Framebuffer&, VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void nextSubpass(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void end();

	size_t getSubpassCount() const { return subpass_count; }
	const std::vector<RenderTargetInfo>& getRenderTargetInfo() const { return render_targets; }
	operator const VkRenderPass& () const { return render_pass; }

private:
	VkRenderPass render_pass = nullptr;
	size_t subpass_count = 0;
	size_t current_subpass = 0;
	std::vector<RenderTargetInfo> render_targets{};
	std::vector<VkClearValue> clear_values{};
};