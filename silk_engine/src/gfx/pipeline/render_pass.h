#pragma once

#include "subpass.h"

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

	size_t getSubpassCount() const { return subpasses.size(); }
	operator const VkRenderPass& () const { return render_pass; }

private:
	VkRenderPass render_pass;
	std::vector<Subpass> subpasses;
};