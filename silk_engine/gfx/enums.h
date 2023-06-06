#pragma once

enum class ImageType : std::underlying_type_t<VkImageType>
{
	_1D = VK_IMAGE_TYPE_1D,
	_2D = VK_IMAGE_TYPE_2D,
	_3D = VK_IMAGE_TYPE_3D
};

enum class ImageViewType : std::underlying_type_t<VkImageViewType>
{
	_1D = VK_IMAGE_VIEW_TYPE_1D,
	_2D = VK_IMAGE_VIEW_TYPE_2D,
	_3D = VK_IMAGE_VIEW_TYPE_3D,
	CUBE = VK_IMAGE_VIEW_TYPE_CUBE,
	_1D_ARRAY = VK_IMAGE_VIEW_TYPE_1D_ARRAY,
	_2D_ARRAY = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
	CUBE_ARRAY = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
};

static constexpr ImageType getImageViewTypeImageType(ImageViewType image_view_type)
{
	switch (image_view_type)
	{
	case ImageViewType::_1D:
	case ImageViewType::_1D_ARRAY:
		return ImageType::_1D;
	case ImageViewType::_2D:
	case ImageViewType::_2D_ARRAY:
	case ImageViewType::CUBE:
	case ImageViewType::CUBE_ARRAY:
		return ImageType::_2D;
	case ImageViewType::_3D:
		return ImageType::_3D;
	}
	return ImageType::_2D;
}

enum class ImageAspect : VkImageAspectFlags
{
	COLOR = VK_IMAGE_ASPECT_COLOR_BIT,
	DEPTH = VK_IMAGE_ASPECT_DEPTH_BIT,
	STENCIL = VK_IMAGE_ASPECT_STENCIL_BIT
};
ADD_ENUM_CLASS_OPERATORS(ImageAspect);

enum class Format : std::underlying_type_t<VkFormat>
{
	RED = VK_FORMAT_R8_UNORM,
	RG = VK_FORMAT_R8G8_UNORM,
	RGB = VK_FORMAT_R8G8B8_UNORM,
	RGBA = VK_FORMAT_R8G8B8A8_UNORM,
	BGRA = VK_FORMAT_B8G8R8A8_UNORM,
	DEPTH16 = VK_FORMAT_D16_UNORM,
	DEPTH = VK_FORMAT_D32_SFLOAT,
	STENCIL = VK_FORMAT_S8_UINT,
	DEPTH16_STENCIL = VK_FORMAT_D16_UNORM_S8_UINT,
	DEPTH24_STENCIL = VK_FORMAT_D24_UNORM_S8_UINT,
	DEPTH_STENCIL = VK_FORMAT_D32_SFLOAT_S8_UINT
};

static constexpr bool isStencilFormat(Format format) { return format >= Format::STENCIL && format <= Format::DEPTH_STENCIL; }
static constexpr bool isDepthFormat(Format format) { return format >= Format::DEPTH16 && format <= Format::DEPTH_STENCIL && format != Format::STENCIL; }
static constexpr bool isDepthStencilFormat(Format format) { return format >= Format::DEPTH16 && format <= Format::DEPTH_STENCIL; }
static constexpr bool isStencilOnlyFormat(Format format) { return format == Format::STENCIL; }
static constexpr bool isDepthOnlyFormat(Format format) { return format == Format::DEPTH || format == Format::DEPTH16; }
static constexpr bool isDepthStencilOnlyFormat(Format format) { return format == Format::DEPTH24_STENCIL || format == Format::DEPTH16_STENCIL; }
static constexpr bool isColorFormat(Format format) { return !isDepthStencilFormat(format); }
static constexpr ImageAspect getFormatImageAspect(Format format) { return (isColorFormat(format) * ImageAspect::COLOR) | (isDepthFormat(format) * ImageAspect::DEPTH) | (isStencilFormat(format) * ImageAspect::STENCIL); }
static constexpr uint32_t getFormatChannels(Format format)
{
	switch (format)
	{
	case Format::RED: return 1;
	case Format::RG: return 2;
	case Format::RGB: return 3;
	case Format::RGBA: return 4;
	case Format::BGRA: return 4;
	case Format::DEPTH16:
	case Format::DEPTH:
	case Format::STENCIL:
	case Format::DEPTH16_STENCIL:
	case Format::DEPTH24_STENCIL:
	case Format::DEPTH_STENCIL:  // NOTE: Might be 2, probably not gonna use this anyways tho
		return 1;
	}
	return 4;
}

static constexpr Format getChannelsFormat(uint32_t channels)
{
	switch (channels)
	{
	case 1: return Format::RED;
	case 2: return Format::RG;
	case 3: return Format::RGB;
	case 4: return Format::RGBA;
	}
	return Format::BGRA;
}

static constexpr size_t getFormatSize(Format format)
{
	switch (format)
	{
	case Format::RED: return 1;
	case Format::RG: return 2;
	case Format::RGB: return 3;
	case Format::RGBA: return 4;
	case Format::BGRA: return 4;
	case Format::DEPTH16: return 2;
	case Format::DEPTH: return 4;
	case Format::STENCIL: return 1;
	case Format::DEPTH16_STENCIL: return 3;
	case Format::DEPTH24_STENCIL: return 4;
	case Format::DEPTH_STENCIL: return 5;
	}
	return 4;
}

enum class ImageUsage : VkImageUsageFlags
{
	TRANSFER_SRC = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	TRANSFER_DST = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	SAMPLED = VK_IMAGE_USAGE_SAMPLED_BIT,
	STORAGE = VK_IMAGE_USAGE_STORAGE_BIT,
	COLOR_ATTACHMENT = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	TRANSIENT_ATTACHMENT = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	INPUT_ATTACHMENT = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
};
ADD_ENUM_CLASS_OPERATORS(ImageUsage);

enum class PipelineStage : VkPipelineStageFlags
{
	TOP = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	DRAW_INDIRECT = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	VERTEX_INPUT = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	VERTEX = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	TESSELATION_CONTROL = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
	TESSELATION_EVALUATION = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
	GEOMETRY = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
	FRAGMENT = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	EARLY_FRAGMENT_TESTS = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	LATE_FRAGMENT_TESTS = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
	COLOR_ATTACHMENT_OUTPUT = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	COMPUTE = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	TRANSFER = VK_PIPELINE_STAGE_TRANSFER_BIT,
	BOTTOM = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	HOST = VK_PIPELINE_STAGE_HOST_BIT,
	ALL_GRAPHICS = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
	ALL_COMMANDS = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	NONE = VK_PIPELINE_STAGE_NONE,
	TRANSFORM_FEEDBACK = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
	CONDITIONAL_RENDERING = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
	ACCELERATION_STRUCTURE_BUILD = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
	RAY_TRACING = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
	FRAGMENT_DENSITY_PROCESS = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
	FRAGMENT_SHADING_RATE_ATTACHMENT = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
	COMMAND_PREPROCESS = VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV,
	TASK = VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
	MESH = VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT,
	SHADING_RATE_IMAGE = VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV
};
ADD_ENUM_CLASS_OPERATORS(PipelineStage);

enum class MipmapMode : std::underlying_type_t<VkSamplerMipmapMode>
{
	NEAREST = VK_SAMPLER_MIPMAP_MODE_NEAREST,
	LINEAR = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	NONE, // This is used for image class to determine if using mipmapping or not
};

enum class Wrap : std::underlying_type_t<VkSamplerAddressMode>
{
	REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	MIRROR_REPEAT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	CLAMP_TO_BORDER = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	MIRROR_CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
};

enum class Filter : std::underlying_type_t<VkFilter>
{
	NEAREST = VK_FILTER_NEAREST,
	LINEAR = VK_FILTER_LINEAR
};

enum class ShaderStage : VkShaderStageFlags
{
	VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
	FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
	GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT,
	COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT,
	TESSELATION_CONTROL = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
	TESSELATION_EVALUATION = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
	RAYGEN = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
	ANY_HIT = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
	CLOSEST_HIT = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
	MISS = VK_SHADER_STAGE_MISS_BIT_KHR,
	INTERSECTION = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
	CALLABLE = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
	TASK = VK_SHADER_STAGE_TASK_BIT_EXT,
	MESH = VK_SHADER_STAGE_MESH_BIT_EXT
};
ADD_ENUM_CLASS_OPERATORS(ShaderStage);