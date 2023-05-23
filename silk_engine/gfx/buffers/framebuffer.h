#pragma once

#include "silk_engine/gfx/images/image.h"

class SwapChain;
class RenderPass;

class Framebuffer : NoCopy
{
public:
	Framebuffer(const SwapChain& swap_chain, const RenderPass& render_pass, uint32_t width, uint32_t height, bool imageless = false);
	~Framebuffer();

	const RenderPass& getRenderPass() const { return render_pass; }
	const SwapChain& getSwapChain() const { return swap_chain; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	const std::vector<shared<Image>>& getAttachments() const;
	operator const VkFramebuffer& () const;

private:
	std::vector<VkFramebuffer> framebuffers;
	const RenderPass& render_pass;
	uint32_t width = 0;
	uint32_t height = 0; 
	std::vector<std::vector<shared<Image>>> attachments;
	const SwapChain& swap_chain;
};