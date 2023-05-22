#pragma once

#include "gfx/images/image.h"

class Pass;

class Resource
{
public:
	enum class Type
	{
		BUFFER,
		ATTACHMENT
	};

public:
	Resource(const char* name, Type type, Pass& pass) 
		: name(name), type(type), pass(pass) {}

	const char* getName() const { return name; }
	Type getType() const { return type; }
	Pass& getPass() { return pass; }
	const Pass& getPass() const { return pass; }
	const shared<Image>& getAttachment() const;

	void setClearColor(const VkClearColorValue& clear_color) { attachment.clear = VkClearValue{ .color = clear_color }; }
	void setClearDepthStencil(const VkClearDepthStencilValue& clear_depth_stencil) { attachment.clear = VkClearValue{ .depthStencil = clear_depth_stencil }; }

private:
	const char* name;
	Type type;
	Pass& pass;

public:
	struct AttachmentInfo
	{
		Image::Format format = Image::Format::BGRA;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		std::optional<VkClearValue> clear = std::nullopt;
		size_t index = 0;
	} attachment = {};
};