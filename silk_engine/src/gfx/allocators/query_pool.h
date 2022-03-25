#pragma once

#include <vulkan/vulkan.hpp>

class QueryPool
{
public:
	QueryPool(vk::QueryType query_type = vk::QueryType::eOcclusion, uint32_t query_count = 1, vk::QueryPipelineStatisticFlags pipeline_statistic_flags = vk::QueryPipelineStatisticFlags(0));
	~QueryPool();

	void begin(uint32_t index = 0, vk::QueryControlFlags flags = vk::QueryControlFlagBits::ePrecise);
	void end(uint32_t index = 0);

	uint64_t getResult(uint32_t index = 0, bool wait = false);

	operator const vk::QueryPool& () const { return query_pool; }

private:
	vk::QueryPool query_pool;
	vk::QueryType query_type;
	vk::QueryPipelineStatisticFlags pipeline_statistic_flags;
	std::vector<vk::Bool32> queries;
	std::vector<uint64_t> results;
};