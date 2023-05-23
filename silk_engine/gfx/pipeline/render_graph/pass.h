#pragma once

#include "silk_engine/gfx/images/image.h"

class Resource;
class RenderGraph;
class RenderPass;

class Pass
{
public:
	enum class Type
	{
		RENDER,
		COMPUTE
	};

public:
	Pass(const char* name, Type type, RenderGraph& render_graph)
		: name(name), type(type), render_graph(render_graph) {}
	
	Resource& addAttachment(const char* name, Image::Format format = Image::Format::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, const std::vector<Resource*>& inputs = {});
	void setRenderCallback(std::function<void(const RenderGraph& render_graph)>&& render_callback) { this->render_callback = std::move(render_callback); }
	void setSubpass(uint32_t subpass) { render.subpass = subpass; }

	const std::vector<Resource*>& getInputs() const { return inputs; }
	const std::vector<Resource*>& getOutputs() const { return outputs; }
	const RenderPass& getRenderPass() const;
	uint32_t getSubpass() const { return render.subpass; }

	void callRender() const { render_callback(render_graph); }
	
private:
	const char* name;
	Type type;
	RenderGraph& render_graph;
	std::vector<Resource*> inputs;
	std::vector<Resource*> outputs;

	std::function<void(const RenderGraph&)> render_callback;

private:
	struct Render
	{
		uint32_t subpass = 0;
	} render;
};