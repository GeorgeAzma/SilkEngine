#pragma once

class QueryPool
{
public:
	QueryPool(VkQueryType query_type = VK_QUERY_TYPE_OCCLUSION, uint32_t query_count = 1);
	QueryPool(VkQueryPipelineStatisticFlags pipeline_statistic_flags, uint32_t query_count = 1);
	~QueryPool();

	void begin(uint32_t index = 0);
	void end(uint32_t index = 0);

	uint64_t getOcclusionResult(uint32_t index = 0, bool wait = false);
	std::vector<uint32_t> getResults(uint32_t index = 0, bool wait = false);

	operator const VkQueryPool& () const { return query_pool; }

private:
	VkQueryPool query_pool = nullptr;
	VkQueryType query_type;
	VkQueryPipelineStatisticFlags pipeline_statistic_flags;
	std::vector<VkBool32> queries;
};