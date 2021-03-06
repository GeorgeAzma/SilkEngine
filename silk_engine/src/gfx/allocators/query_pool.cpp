#include "query_pool.h"
#include "gfx/graphics.h"
#include "utils/math.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/command_buffer.h"

QueryPool::QueryPool(VkQueryType query_type, uint32_t query_count, VkQueryPipelineStatisticFlags pipeline_statistic_flags)
	: query_type(query_type), pipeline_statistic_flags(pipeline_statistic_flags), queries(query_count, VK_FALSE)
{
	VkQueryPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	ci.queryType = query_type;
	ci.queryCount = query_count;
	ci.pipelineStatistics = pipeline_statistic_flags;
	query_pool = Graphics::logical_device->createQueryPool(ci);
}

QueryPool::~QueryPool()
{
	Graphics::logical_device->destroyQueryPool(query_pool);
}

void QueryPool::begin(uint32_t index, VkQueryControlFlags flags)
{
	if (queries[index])
		return;
	Graphics::logical_device->resetQueryPool(query_pool, index, 1);
	Graphics::getActiveCommandBuffer().beginQuery(query_pool, index, flags);
}

void QueryPool::end(uint32_t index)
{
	if (!queries[index])
		return;
	Graphics::getActiveCommandBuffer().endQuery(query_pool, index);
}

uint64_t QueryPool::getResult(uint32_t index, bool wait)
{
	size_t data_count = 1;

	switch (query_type)
	{
	case VK_QUERY_TYPE_OCCLUSION:
		data_count = 1;
		break;
	case VK_QUERY_TYPE_PIPELINE_STATISTICS:
		data_count = math::getEnabledFlagsCount((const VkQueryPipelineStatisticFlags&)pipeline_statistic_flags);		
		break;
	case VK_QUERY_TYPE_TIMESTAMP:
		data_count = 1;
		break;
	}
	uint32_t data_size = data_count * sizeof(uint64_t);

	results = (std::vector<uint64_t>)Graphics::logical_device->getQueryPoolResults<uint64_t>(query_pool, index, 1, data_size, data_size, VK_QUERY_RESULT_64_BIT | (wait * VK_QUERY_RESULT_WAIT_BIT));

	return results.front();
}
