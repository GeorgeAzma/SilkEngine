#pragma once

#include <vulkan/vulkan.hpp>

class QueryPool
{
public:
	QueryPool(vk::QueryType query_type = vk::QueryType::eOcclusion, vk::QueryPipelineStatisticFlags pipeline_statistic_flags = vk::QueryPipelineStatisticFlags(0));
	~QueryPool();

	void begin(uint32_t index = 0);
	void end(uint32_t index = 0);

	std::vector<uint64_t> getResults();

	operator const vk::QueryPool& () const { return query_pool; }

private:
	vk::QueryPool query_pool;
	vk::QueryType query_type;
	vk::QueryPipelineStatisticFlags pipeline_statistic_flags;
};