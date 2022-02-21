#pragma once

#include "pipeline.h"

class WindowResizeEvent;

class ComputePipeline : public Pipeline
{
public:
	ComputePipeline& setShader(shared<Shader> shader);

	void build();
	void bind();
	
private:
	void create() override;

private:
	VkComputePipelineCreateInfo create_info{};
};