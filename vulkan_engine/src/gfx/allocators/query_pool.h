#pragma once

class QueryPool
{
public:
	QueryPool(VkQueryType query_type = VK_QUERY_TYPE_OCCLUSION, VkQueryPipelineStatisticFlags pipeline_statistic_flags = 0);
	~QueryPool();

	void begin(uint32_t index = 0);
	void end(uint32_t index = 0);

	std::vector<uint64_t> getResults();

	operator const VkQueryPool& () const { return query_pool; }

private:
	VkQueryPool query_pool;
	VkQueryType query_type;
	VkQueryPipelineStatisticFlags pipeline_statistic_flags;
};