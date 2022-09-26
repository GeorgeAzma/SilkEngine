#pragma once

#include "gfx/images/image.h"
#include "core/event.h"

class Framebuffer : NonCopyable
{
public:
	Framebuffer(VkRenderPass render_pass, uint32_t width = 0, uint32_t height = 0);
	~Framebuffer();

	Framebuffer& addAttachment(const Image::Props& image_props);
	Framebuffer& addAttachment(Image::Format format = Image::Format::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	//For adding swap chain image attachment ONLY
	Framebuffer& addAttachment(const std::vector<shared<Image>>& images);
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
};