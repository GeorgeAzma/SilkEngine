#pragma once

#include "gfx/images/image.h"

class SwapChain;

class Framebuffer : NonCopyable
{
public:
	Framebuffer(const SwapChain& swap_chain, VkRenderPass render_pass, uint32_t width, uint32_t height);
	~Framebuffer();

	Framebuffer& addAttachment(Image::Props image_props);
	Framebuffer& addAttachment(Image::Format format = Image::Format::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	Framebuffer& addSwapchainAttachments();
	void build(bool imageless = false);

	const VkRenderPass& getRenderPass() const { return render_pass; }
	const SwapChain& getSwapChain() const { return swap_chain; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	const std::vector<shared<Image>>& getAttachments() const;
	operator const VkFramebuffer& () const;

private:
	std::vector<VkFramebuffer> framebuffers;
	VkRenderPass render_pass = nullptr;
	uint32_t width = 0;
	uint32_t height = 0; 
	std::vector<std::vector<shared<Image>>> attachments;
	const SwapChain& swap_chain;
};