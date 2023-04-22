#pragma once

#include "pipeline.h"

class ComputePipeline : public Pipeline
{
public:
	ComputePipeline(const shared<Shader>& shader, const std::vector<Constant>& constants = {});

	void bind();
	void dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y = 1, uint32_t global_invocation_count_z = 1) const;

	void create() override;

private:
	VkComputePipelineCreateInfo ci{};

public:
	static shared<ComputePipeline> get(std::string_view name) { return compute_pipelines.at(name); }
	static void add(std::string_view name, const shared<ComputePipeline> compute_pipeline) { compute_pipelines.insert_or_assign(name, compute_pipeline); }
	static void destroy() { compute_pipelines.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<ComputePipeline>> compute_pipelines{};
};