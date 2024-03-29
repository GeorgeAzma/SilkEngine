#include "query_pool.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/command_queue.h"

QueryPool::QueryPool(QueryType query_type, uint32_t query_count)
	: query_type(query_type), queries(query_count, false)
{
	VkQueryPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	ci.queryType = VkQueryType(query_type);
	ci.queryCount = query_count;
	query_pool = RenderContext::getLogicalDevice().createQueryPool(ci);
}

QueryPool::QueryPool(PipelineStatistics pipeline_statistics, uint32_t query_count)
	: query_type(PIPELINE_STATISTICS), pipeline_statistics(pipeline_statistics), queries(query_count, false)
{
	VkQueryPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	ci.queryType = VkQueryType(query_type);
	ci.queryCount = query_count;
	ci.pipelineStatistics = VkQueryPipelineStatisticFlags(pipeline_statistics);
	query_pool = RenderContext::getLogicalDevice().createQueryPool(ci);
}

QueryPool::~QueryPool()
{
	RenderContext::getLogicalDevice().destroyQueryPool(query_pool);
}

void QueryPool::begin(uint32_t index)
{
	if (queries[index])
		return;
	RenderContext::getCommandBuffer().resetQueryPool(query_pool, index, 1);
	RenderContext::getCommandBuffer().beginQuery(query_pool, index, query_type == OCCLUSION ? VK_QUERY_CONTROL_PRECISE_BIT : 0);
	queries[index] = true;
}

void QueryPool::end(uint32_t index)
{
	if (!queries[index])
		return;
	RenderContext::getCommandBuffer().endQuery(query_pool, index); 
	queries[index] = false;
}

uint64_t QueryPool::getOcclusionResult(uint32_t index, bool wait)
{
	std::vector<uint32_t> results = RenderContext::getLogicalDevice().getQueryPoolResults(query_pool, index, 1, sizeof(uint64_t), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | (wait * VK_QUERY_RESULT_WAIT_BIT));
	uint64_t result = 0;
	memcpy(&result, results.data(), sizeof(uint64_t));
	return result;
}

std::vector<uint32_t> QueryPool::getResults(uint32_t index, bool wait)
{
	size_t data_count = 1;
	switch (query_type)
	{
	case OCCLUSION:
	case TIMESTAMP:
		data_count = 1;
		break;
	case PIPELINE_STATISTICS:
		data_count = std::popcount((const VkQueryPipelineStatisticFlags&)pipeline_statistics);
		break;
	}
	uint32_t data_size = data_count * sizeof(uint32_t);
	return RenderContext::getLogicalDevice().getQueryPoolResults(query_pool, index, 1, data_size, data_size, wait * VK_QUERY_RESULT_WAIT_BIT);
}
