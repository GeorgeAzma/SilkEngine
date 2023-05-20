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
	VkAttachmentStoreOp store_operation = VK_ATTACHMENT_STORE_OP_STORE;
	VkAttachmentLoadOp stencil_load_operation = VK_ATTACHMENT_LOAD_OP_MAX_ENUM; // VK_ATTACHMENT_LOAD_OP_MAX_ENUM sets stencil load_operation to same thing as load_operation if format is a stencil/depth_stencil format
	VkAttachmentStoreOp stencil_store_operation = VK_ATTACHMENT_STORE_OP_MAX_ENUM; // VK_ATTACHMENT_STORE_OP_MAX_ENUM sets stencil store_operation to same thing as store_operation if format is a stencil/depth_stencil format
	VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	bool preserve = false;
};

class RenderPass : NoCopy
{
	struct SubpassInfo
	{
		std::vector<VkAttachmentReference> resolve_attachment_references;
		std::vector<VkAttachmentReference> color_attachment_references;
		VkAttachmentReference depth_attachment_reference = { VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED };
		std::vector<uint32_t> preserve_attachment_references;
		std::vector<VkAttachmentReference> input_attachment_references;
	};

public:
	RenderPass() { addSubpass(); }
	~RenderPass();

	void addSubpass();
	void addInputAttachment(uint32_t index);
	size_t addAttachment(const AttachmentProps& attachment_props);

	void addSubpassDependency(const VkSubpassDependency& dependency);

	void build();

	void begin(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void nextSubpass(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE);
	void end();

	void setViewport(const ivec2& viewport) { this->viewport = viewport; }
	void setClearValue(size_t index, const VkClearValue& clear_value) { clear_values[index] = clear_value; }
	void resize(const SwapChain& swap_chain);

	bool isInputAttachment(uint32_t attachment) const { return attachments_used_as_inputs.contains(attachment); }
	size_t getSubpassCount() const { return subpass_infos.size(); }
	const std::vector<VkAttachmentDescription>& getAttachmentDescriptions() const { return attachment_descriptions; }
	operator const VkRenderPass& () const { return render_pass; }
	const shared<Framebuffer>& getFramebuffer() const { return framebuffer; }

private:
	VkRenderPass render_pass = nullptr;
	std::vector<SubpassInfo> subpass_infos; 
	std::vector<VkSubpassDependency> subpass_dependencies;
	std::vector<VkAttachmentDescription> attachment_descriptions;
	std::unordered_set<uint32_t> attachments_used_as_inputs;
	std::vector<VkClearValue> clear_values;
	bool any_cleared = false;
	ivec2 viewport = ivec2(0);
	shared<Framebuffer> framebuffer = nullptr;
};