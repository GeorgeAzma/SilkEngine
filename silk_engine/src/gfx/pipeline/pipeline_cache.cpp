#include "pipeline_cache.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "io/file.h"

bool PipelineCache::Header::isValid() const
{
	const VkPhysicalDeviceProperties& props = Graphics::physical_device->getProperties(); 
    if (length <= 0 || version != VK_PIPELINE_CACHE_HEADER_VERSION_ONE || vendor != props.vendorID || device != props.deviceID || memcmp(uuid, props.pipelineCacheUUID, sizeof(uuid)) != 0)
        return false;
	return true;
}

PipelineCache::PipelineCache()
{
	VkPipelineCacheCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	std::vector<uint8_t> cache = File::read("res/cache/pipeline_cache.bin", std::ios::binary);
	if (cache.size())
	{
        Header header;
        memcpy(&header.length, cache.data() + offsetof(Header, length), sizeof(header.length));
        memcpy(&header.version, cache.data() + offsetof(Header, version), sizeof(header.version));
        memcpy(&header.vendor, cache.data() + offsetof(Header, vendor), sizeof(header.vendor));
        memcpy(&header.device, cache.data() + offsetof(Header, device), sizeof(header.device));
        memcpy(header.uuid, cache.data() + offsetof(Header, uuid), sizeof(header.uuid));
		if (header)
		{
			ci.initialDataSize = cache.size();
			ci.pInitialData = cache.data();
		}
	}
	pipeline_cache = Graphics::logical_device->createPipelineCache(ci);
	if(!pipeline_cache)
		SK_WARN("Couldn't create pipeline cache");
}

PipelineCache::~PipelineCache()
{
	if (!pipeline_cache)
		return;
	std::vector<uint8_t> data = Graphics::logical_device->getPipelineCacheData(pipeline_cache);
	std::ofstream cache("res/cache/pipeline_cache.bin", std::ios::binary);
	cache.write((const char*)data.data(), data.size());
	Graphics::logical_device->destroyPipelineCache(pipeline_cache);
}