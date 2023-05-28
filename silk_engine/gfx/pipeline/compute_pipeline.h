#pragma once

#include "pipeline.h"

class ComputePipeline : public Pipeline
{
public:
	ComputePipeline(const shared<Shader>& shader, const std::vector<Constant>& constants = {});

	void bind() override;
	void dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y = 1, uint32_t global_invocation_count_z = 1);

	void create() override;

private:
	VkComputePipelineCreateInfo ci{};

public:
	static shared<ComputePipeline> get(std::string_view name) { if (auto it = compute_pipelines.find(name); it != compute_pipelines.end()) return it->second; else return nullptr; }
	static shared<ComputePipeline> add(std::string_view name, const shared<ComputePipeline> compute_pipeline) { return compute_pipelines.insert_or_assign(name, compute_pipeline).first->second; }
	static void destroy() { compute_pipelines.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<ComputePipeline>> compute_pipelines{};
};