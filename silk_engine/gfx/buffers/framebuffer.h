#pragma once

#include "gfx/images/image.h"

class SwapChain;

class Framebuffer : NonCopyable
{
public:
	Framebuffer(const SwapChain& swap_chain, VkRenderPass render_pass, uint32_t width, uint32_t height);
	~Framebuffer();

	Framebuffer& addAttachment(const Image::Props& image_props);
	Framebuffer& addAttachment(Image::Format format = Image::Format::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	Framebuffer& addSwapchainAttachments();
	void build();

	const std::vector<shared<Image>>& getAttachments() const;
	operator const VkFramebuffer& () const;

private:
	std::vector<VkFramebuffer> framebuffers;
	VkRenderPass render_pass = nullptr;
	uint32_t width = 0;
	uint32_t height = 0; 
	bool multisampled = false;
	std::vector<std::vector<shared<Image>>> attachments;
	const SwapChain& swap_chain;
};