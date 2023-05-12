#pragma once


struct VulkanObject
{
	enum class Type : std::underlying_type_t<VkObjectType>
	{
       UNKNOWN = 0,
       INSTANCE = 1,
       PHYSICAL_DEVICE = 2,
       DEVICE = 3,
       QUEUE = 4,
       SEMAPHORE = 5,
       COMMAND_BUFFER = 6,
       FENCE = 7,
       DEVICE_MEMORY = 8,
       BUFFER = 9,
       IMAGE = 10,
       EVENT = 11,
       QUERY_POOL = 12,
       BUFFER_VIEW = 13,
       IMAGE_VIEW = 14,
       SHADER_MODULE = 15,
       PIPELINE_CACHE = 16,
       PIPELINE_LAYOUT = 17,
       RENDER_PASS = 18,
       PIPELINE = 19,
       DESCRIPTOR_SET_LAYOUT = 20,
       SAMPLER = 21,
       DESCRIPTOR_POOL = 22,
       DESCRIPTOR_SET = 23,
       FRAMEBUFFER = 24,
       COMMAND_POOL = 25,
       SAMPLER_YCBCR_CONVERSION = 1000156000,
       DESCRIPTOR_UPDATE_TEMPLATE = 1000085000,
       PRIVATE_DATA_SLOT = 1000295000,
       SURFACE = 1000000000, // KHR
       SWAPCHAIN = 1000001000, // KHR
       DISPLAY = 1000002000, // KHR
       DISPLAY_MODE = 1000002001, // KHR
       DEBUG_REPORT_CALLBACK = 1000011000, // EXT
#ifdef VK_ENABLE_BETA_EXTENSIONS
        VIDEO_SESSION = 1000023000, // KHR
        VIDEO_SESSION_PARAMETERS = 1000023001, // KHR
#endif
        CU_MODULE = 1000029000, // NVX
        CU_FUNCTION = 1000029001, // NVX
        DEBUG_UTILS_MESSENGER = 1000128000, // EXT
        ACCELERATION_STRUCTURE = 1000150000, // KHR
        VALIDATION_CACHE = 1000160000, // EXT
        ACCELERATION_STRUCTURE_NV = 1000165000, // NV
        PERFORMANCE_CONFIGURATION_INTEL = 1000210000, // INTEL
        DEFERRED_OPERATION = 1000268000, // KHR
        INDIRECT_COMMANDS_LAYOUT = 1000277000, // NV
        BUFFER_COLLECTION = 1000366000, // FUCHSIA
        MICROMAP = 1000396000, // EXT
        OPTICAL_FLOW_SESSION = 1000464000, // NV
	};
#ifdef SK_ENABLE_DEBUG_OUTPUT
	static void create(Type type = Type::UNKNOWN, uint64_t handle = 0, const char* name = nullptr);
#else
    static void create(Type type = Type::UNKNOWN, uint64_t handle = 0, const char* name = nullptr) {}
#endif
};