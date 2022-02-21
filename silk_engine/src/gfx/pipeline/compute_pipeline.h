#pragma once

#include "pipeline.h"

class WindowResizeEvent;

class ComputePipeline : public Pipeline
{
public:
	ComputePipeline(const std::string& shader_file);

	void bind();
	void dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y = 1, uint32_t global_invocation_count_z = 1) const;
	
private:
	void create() override;

private:
	VkComputePipelineCreateInfo create_info{};
};