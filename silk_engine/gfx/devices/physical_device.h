#pragma once

class Instance;

class PhysicalDevice : NoCopy
{
	friend class LogicalDevice;

public:
	enum class Feature
	{
		ROBUST_BUFFER_ACCESS,
		FULL_DRAW_INDEX_UINT32,
		IMAGE_CUBE_ARRAY,
		INDEPENDENT_BLEND,
		GEOMETRY_SHADER,
		TESSELLATION_SHADER,
		SAMPLE_RATE_SHADING,
		DUAL_SRC_BLEND,
		LOGIC_OP,
		MULTI_DRAW_INDIRECT,
		DRAW_INDIRECT_FIRST_INSTANCE,
		DEPTH_CLAMP,
		DEPTH_BIAS_CLAMP,
		FILL_MODE_NON_SOLID,
		DEPTH_BOUNDS,
		WIDE_LINES,
		LARGE_POINTS,
		ALPHA_TO_ONE,
		MULTI_VIEWPORT,
		SAMPLER_ANISOTROPY,
		TEXTURE_COMPRESSION_ETC2,
		TEXTURE_COMPRESSION_ASTC_LDR,
		TEXTURE_COMPRESSION_BC,
		OCCLUSION_QUERY_PRECISE,
		PIPELINE_STATISTICS_QUERY,
		VERTEX_PIPELINE_STORES_AND_ATOMICS,
		FRAGMENT_STORES_AND_ATOMICS,
		SHADER_TESSELLATION_AND_GEOMETRY_POINT_SIZE,
		SHADER_IMAGE_GATHER_EXTENDED,
		SHADER_STORAGE_IMAGE_EXTENDED_FORMATS,
		SHADER_STORAGE_IMAGE_MULTISAMPLE,
		SHADER_STORAGE_IMAGE_READ_WITHOUT_FORMAT,
		SHADER_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT,
		SHADER_UNIFORM_BUFFER_ARRAY_DYNAMIC_INDEXING,
		SHADER_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING,
		SHADER_STORAGE_BUFFER_ARRAY_DYNAMIC_INDEXING,
		SHADER_STORAGE_IMAGE_ARRAY_DYNAMIC_INDEXING,
		SHADER_CLIP_DISTANCE,
		SHADER_CULL_DISTANCE,
		SHADER_FLOAT64,
		SHADER_INT64,
		SHADER_INT16,
		SHADER_RESOURCE_RESIDENCY,
		SHADER_RESOURCE_MIN_LOD,
		SPARSE_BINDING,
		SPARSE_RESIDENCY_BUFFER,
		SPARSE_RESIDENCY_IMAGE_2D,
		SPARSE_RESIDENCY_IMAGE_3D,
		SPARSE_RESIDENCY_2_SAMPLES,
		SPARSE_RESIDENCY_4_SAMPLES,
		SPARSE_RESIDENCY_8_SAMPLES,
		SPARSE_RESIDENCY_16_SAMPLES,
		SPARSE_RESIDENCY_ALIASED,
		VARIABLE_MULTISAMPLE_RATE,
		INHERITED_QUERIES,
		VULKAN10_LAST = INHERITED_QUERIES,

		STORAGE_BUFFER_16BIT_ACCESS,
		UNIFORM_AND_STORAGE_BUFFER_16BIT_ACCESS,
		STORAGE_PUSH_CONSTANT_16,
		STORAGE_INPUT_OUTPUT_16,
		MULTIVIEW,
		MULTIVIEW_GEOMETRY_SHADER,
		MULTIVIEW_TESSELATION_SHADER,
		VARIABLE_POINTERS_STORAGE_BUFFER,
		VARIABLE_POINTERS,
		PROTECTED_MEMORY,
		SAMPLER_YCBR_CONVERSION,
		SHADER_DRAW_PARAMETERS,
		VULKAN11_LAST = SHADER_DRAW_PARAMETERS,

		SAMPLER_MIRROR_CLAMP_TO_EDGE,
		DRAW_INDIRECT_COUNT,
		STORAGE_BUFFER_8BIT_ACCESS,
		UNIFORM_AND_STORAGE_BUFFER_8BIT_ACCESS,
		STORAGE_PUSH_CONSTANT_8,
		SHADER_BUFFER_INT64_ATOMICS,
		SHADER_SHARED_INT64_ATOMICS,
		SHADER_FLOAT16,
		SHADER_INT8,
		DESCRIPTOR_INDEXING,
		SHADER_INPUT_ATTACHMENT_ARRAY_DYNAMIC_INDEXING,
		SHADER_UNIFORM_TEXEL_BUFFER_ARRAY_DYNAMIC_INDEXING,
		SHADER_STORAGE_TEXEL_BUFFER_ARRAY_DYNAMIC_INDEXING,
		SHADER_UNIFORM_BUFFER_ARRAY_NON_UNIFORM_INDEXING,
		SHADER_SAMPLED_IMAGE_ARRAY_NON_UNIFORM_INDEXING,
		SHADER_STORAGE_BUFFER_ARRAY_NON_UNIFORM_INDEXING,
		SHADER_STORAGE_IMAGE_ARRAY_NON_UNIFORM_INDEXING,
		SHADER_INPUT_ATTACHMENT_ARRAY_NON_UNIFORM_INDEXING,
		SHADER_UNIFORM_TEXEL_BUFFER_ARRAY_NON_UNIFORM_INDEXING,
		SHADER_STORAGE_TEXEL_BUFFER_ARRAY_NON_UNIFORM_INDEXING,
		DESCRIPTOR_BINDING_UNIFORM_BUFFER_UPDATE_AFTER_BIND,
		DESCRIPTOR_BINDING_SAMPLED_IMAGE_UPDATE_AFTER_BIND,
		DESCRIPTOR_BINDING_STORAGE_IMAGE_UPDATE_AFTER_BIND,
		DESCRIPTOR_BINDING_STORAGE_BUFFER_UPDATE_AFTER_BIND,
		DESCRIPTOR_BINDING_UNIFORM_TEXEL_BUFFER_UPDATE_AFTER_BIND,
		DESCRIPTOR_BINDING_STORAGE_TEXEL_BUFFER_UPDATE_AFTER_BIND,
		DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING,
		DESCRIPTOR_BINDING_PARTIALLY_BOUND,
		DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT,
		RUNTIME_DESCRIPTOR_ARRAY,
		SAMPLER_FILTER_MINMAX,
		SCALAR_BLOCK_LAYOUT,
		IMAGELESS_FRAMEBUFFER,
		UNIFORM_BUFFER_STANDARD_LAYOUT,
		SHADER_SUBGROUP_EXTENDED_TYPES,
		SEPARATE_DEPTH_STENCIL_LAYOUTS,
		HOST_QUERY_RESET,
		TIMELINE_SEMAPHORE,
		BUFFER_DEVICE_ADDRESS,
		BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY,
		BUFFER_DEVICE_ADDRESS_MULTI_DEVICE,
		VULKAN_MEMORY_MODEL,
		VULKAN_MEMORY_MODEL_DEVICE_SCOPE,
		VULKAN_MEMORY_MODEL_AVAILABILITY_VISIBILITY_CHAINS,
		SHADER_OUTPUT_VIEWPORT_INDEX,
		SHADER_OUTPUT_LAYER,
		SUBGROUP_BROADCAST_DYNAMIC_ID,
		VULKAN12_LAST = SUBGROUP_BROADCAST_DYNAMIC_ID,

		ROBUST_IMAGE_ACCESS,
		INLINE_UNIFORM_BLOCK,
		DESCRIPTOR_BINDING_INLINE_UNIFORM_BLOCK_UPDATE_AFTER_BIND,
		PIPELINE_CREATION_CACHE_CONTROL,
		PRIVATE_DATA,
		SHADER_DEMOTE_TO_HELPER_INVOCATION,
		SHADER_TERMINATE_INVOCATION,
		SUBGROUP_SIZE_CONTROL,
		COMPUTE_FULL_SUBGROUPS,
		SYNCHRONIZATION2,
		TEXTURE_COMPRESSION_ASTC_HDR,
		SHADER_ZERO_INITIALIZE_WORKGROUP_MEMORY,
		DYNAMIC_RENDERING,
		SHADER_INTEGER_DOT_PRODUCT,
		MAINTENANCE4,
		VULKAN13_LAST = MAINTENANCE4
	};

public:
	PhysicalDevice(const Instance& instance, VkPhysicalDevice physical_device);

	bool supportsExtension(const char* extension_name) const;
	bool supportsFeature(Feature feature) const;
	uint32_t alignSize(uint32_t original_size) const
	{
		size_t min_ubo_alignment = properties.limits.minUniformBufferOffsetAlignment;
		return (min_ubo_alignment > 0) ? ((original_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1)) : original_size;
	}

	VkDevice createLogicalDevice(const VkDeviceCreateInfo& create_info) const;
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	std::vector<VkSurfaceFormatKHR> getSurfaceFormats(VkSurfaceKHR surface) const;
	std::vector<VkPresentModeKHR> getSurfacePresentModes(VkSurfaceKHR surface) const;
	VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface) const;
	bool getSurfaceSupport(uint32_t queue_family_index, VkSurfaceKHR surface) const;

	const std::vector<VkQueueFamilyProperties>& getQueueFamilyProperties() const { return queue_family_properties; }
	const std::vector<uint32_t>& getQueueFamilyIndices() const { return queue_family_indices; }
	const VkPhysicalDeviceProperties& getProperties() const { return properties; }
	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const { return memory_properties; }
	const VkPhysicalDeviceFeatures& getFeatures() const { return features; }
	VkSampleCountFlagBits getMaxSampleCount() const { return max_usable_sample_count; }
	VkFormat getDepthFormat() const { return depth_format; }
	VkFormatProperties getFormatProperties(VkFormat format) const;
	VkImageFormatProperties getImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tilling, VkImageUsageFlags usage, VkImageCreateFlags flags) const;
	int getGraphicsQueue() const { return graphics_queue; }
	int getComputeQueue() const { return compute_queue; }
	int getTransferQueue() const { return transfer_queue; }
	operator const VkPhysicalDevice& () const { return physical_device; }
	const Instance& getInstance() const { return instance; }

private:
	VkPhysicalDevice physical_device = nullptr;
	const Instance& instance;
	std::vector<VkQueueFamilyProperties> queue_family_properties{};
	std::vector<uint32_t> queue_family_indices{}; // Sorted
	std::unordered_map<std::string, uint32_t> supported_extensions{}; // extension name | extension spec version
	int32_t graphics_queue = -1;
	int32_t compute_queue = -1;
	int32_t transfer_queue = -1;
	VkPhysicalDeviceProperties properties{};
	VkPhysicalDeviceMemoryProperties memory_properties{};
	VkPhysicalDeviceFeatures features{};
	VkPhysicalDeviceVulkan11Features vulkan_11_features{};
	VkPhysicalDeviceVulkan12Features vulkan_12_features{};
	VkPhysicalDeviceVulkan13Features vulkan_13_features{};
	VkSampleCountFlagBits max_usable_sample_count = VK_SAMPLE_COUNT_1_BIT;
	VkFormat depth_format = VkFormat(0);
};