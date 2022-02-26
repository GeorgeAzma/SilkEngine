#include "query_pool.h"
#include "gfx/graphics.h"
#include "utils/math.h"
#include "gfx/devices/logical_device.h"

QueryPool::QueryPool(vk::QueryType query_type, vk::QueryPipelineStatisticFlags pipeline_statistic_flags)
	: query_type(query_type), pipeline_statistic_flags()
{
	vk::QueryPoolCreateInfo ci{};
	ci.queryType = query_type;
	ci.queryCount = 1;
	ci.pipelineStatistics = pipeline_statistic_flags;

	Graphics::logical_device->createQueryPool(ci);
}

QueryPool::~QueryPool()
{
	Graphics::logical_device->destroyQueryPool(query_pool);
}

void QueryPool::begin(uint32_t index)
{
	Graphics::logical_device->resetQueryPool(query_pool, index, 1);
	Graphics::active.command_buffer.beginQuery(query_pool, index, vk::QueryControlFlagBits::ePrecise);
}

void QueryPool::end(uint32_t index)
{
	Graphics::active.command_buffer.endQuery(query_pool, index);
}

std::vector<uint64_t> QueryPool::getResults()
{
	size_t data_count = 1;

	switch (query_type)
	{
	case vk::QueryType::eOcclusion:
		data_count = 1;
		break;
	case vk::QueryType::ePipelineStatistics:
		data_count = math::getEnabledFlagsCount((const VkQueryPipelineStatisticFlags&)pipeline_statistic_flags);		
		break;
	case vk::QueryType::eTimestamp:
		data_count = 1;
		break;
	}

	uint32_t data_size = data_count * sizeof(uint64_t);

	std::vector<uint64_t> results = Graphics::logical_device->getQueryPoolResults<uint64_t>(query_pool, 0, 1, data_size, data_size, vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::ePartial);
	Graphics::logical_device->getGraphicsQueue().waitIdle();

	return results;
}
