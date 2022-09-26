#pragma once

#include "pipeline.h"

class ComputePipeline : public Pipeline
{
public:
	ComputePipeline(const shared<Shader>& shader, const std::vector<Constant>& constants = {});

	void bind();
	void dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y = 1, uint32_t global_invocation_count_z = 1) const;
	
private:
	void IsetShader(const shared<Shader>& shader, const std::vector<Constant>& constants) override;
	void create() override; 

private:
	VkComputePipelineCreateInfo ci{};
};