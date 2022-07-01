#pragma once

#include "gfx/images/image2D.h"
#include "core/event.h"

class Framebuffer : NonCopyable
{
public:
	Framebuffer(VkRenderPass render_pass, uint32_t width = 0, uint32_t height = 0);
	~Framebuffer();

	Framebuffer& addAttachment(const Image2DProps& image_props);
	Framebuffer& addAttachment(ImageFormat format = ImageFormat::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	//For adding swap chain image attachment ONLY
	Framebuffer& addAttachment(const std::vector<shared<Image2D>>& images);
	void build();

	const std::vector<shared<Image2D>>& getAttachments() const;
	operator const VkFramebuffer& () const;

private:
	std::vector<VkFramebuffer> framebuffers;
	VkRenderPass render_pass = VK_NULL_HANDLE;
	uint32_t width = 0;
	uint32_t height = 0; 
	bool multisampled = false;
	std::vector<std::vector<shared<Image2D>>> attachments;
};