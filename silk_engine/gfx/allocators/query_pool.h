#pragma once

class QueryPool
{
public:
	QueryPool(VkQueryType query_type = VK_QUERY_TYPE_OCCLUSION, uint32_t query_count = 1, VkQueryPipelineStatisticFlags pipeline_statistic_flags = 0);
	~QueryPool();

	void begin(uint32_t index = 0, VkQueryControlFlags flags = VK_QUERY_CONTROL_PRECISE_BIT);
	void end(uint32_t index = 0);

	uint64_t getResult(uint32_t index = 0, bool wait = false);

	operator const VkQueryPool& () const { return query_pool; }

private:
	VkQueryPool query_pool = nullptr;
	VkQueryType query_type;
	VkQueryPipelineStatisticFlags pipeline_statistic_flags;
	std::vector<VkBool32> queries;
	std::vector<uint64_t> results;
};