#include "query_pool.h"
#include "gfx/graphics.h"
#include "utils/math.h"
#include "gfx/devices/logical_device.h"

QueryPool::QueryPool(VkQueryType query_type, VkQueryPipelineStatisticFlags pipeline_statistic_flags)
	: query_type(query_type), pipeline_statistic_flags()
{
	VkQueryPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	create_info.queryType = query_type;
	create_info.queryCount = 1;
	create_info.pipelineStatistics = pipeline_statistic_flags;

	Graphics::vulkanAssert(vkCreateQueryPool(*Graphics::logical_device, &create_info, nullptr, &query_pool));
}

QueryPool::~QueryPool()
{
	vkDestroyQueryPool(*Graphics::logical_device, query_pool, nullptr);
}

void QueryPool::begin(uint32_t index)
{
	vkResetQueryPool(*Graphics::logical_device, query_pool, index, 1);
	vkCmdBeginQuery(Graphics::active.command_buffer, query_pool, index, VK_QUERY_CONTROL_PRECISE_BIT);
}

void QueryPool::end(uint32_t index)
{
	vkCmdEndQuery(Graphics::active.command_buffer, query_pool, index);
}

std::vector<uint64_t> QueryPool::getResults()
{
	size_t data_count = 1;

	switch (query_type)
	{
	case VK_QUERY_TYPE_OCCLUSION:
		data_count = 1;
		break;
	case VK_QUERY_TYPE_PIPELINE_STATISTICS:
		data_count = math::getEnabledFlagsCount(pipeline_statistic_flags);		
		break;
	case VK_QUERY_TYPE_TIMESTAMP:
		data_count = 1;
		break;
	}

	uint32_t data_size = data_count * sizeof(uint64_t);

	std::vector<uint64_t> results(data_count);
	vkGetQueryPoolResults(*Graphics::logical_device, query_pool, 0, 1, data_size, results.data(), data_size, VK_QUERY_RESULT_PARTIAL_BIT | VK_QUERY_RESULT_64_BIT);
	vkQueueWaitIdle(Graphics::logical_device->getGraphicsQueue());

	return results;
}
