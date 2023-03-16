#pragma once

#include "pipeline.h"
#include "pipeline_stage.h"


class GraphicsPipeline : public Pipeline
{
public:
	enum class EnableTag
	{
		DEPTH_TEST,
		DEPTH_WRITE,
		STENCIL_TEST,
		BLEND,
		SAMPLE_SHADING,
		PRIMITIVE_RESTART,
		RASTERIZER_DISCARD,
		DEPTH_CLAMP,
		DEPTH_BIAS,
		COLOR_BLEND_LOGIC_OP,
	};

	enum class CullMode : VkCullModeFlags
	{
		NONE = VK_CULL_MODE_NONE,
		BACK = VK_CULL_MODE_BACK_BIT,
		FRONT = VK_CULL_MODE_FRONT_BIT,
		FRONT_AND_BACK = VK_CULL_MODE_FRONT_AND_BACK
	};

	enum class PolygonMode
	{
		FILL = VK_POLYGON_MODE_FILL,
		LINE = VK_POLYGON_MODE_LINE,
		POINT = VK_POLYGON_MODE_POINT
	};

	enum class FrontFace
	{
		CLOCKWISE = VK_FRONT_FACE_CLOCKWISE,
		COUNTER_CLOCKWISE = VK_FRONT_FACE_COUNTER_CLOCKWISE
	};

	enum class CompareOp
	{
		ALWAYS = VK_COMPARE_OP_ALWAYS,
		EQUAL = VK_COMPARE_OP_EQUAL,
		GREATER = VK_COMPARE_OP_GREATER,
		GREATER_OR_EQUAL = VK_COMPARE_OP_GREATER_OR_EQUAL,
		LESS = VK_COMPARE_OP_LESS,
		LESS_OR_EQUAL = VK_COMPARE_OP_LESS_OR_EQUAL,
		NEVER = VK_COMPARE_OP_NEVER
	};

	enum class BlendFactor
	{
		ZERO = VK_BLEND_FACTOR_ZERO,
		ONE = VK_BLEND_FACTOR_ONE,
		SRC_COLOR = VK_BLEND_FACTOR_SRC_COLOR,
		ONE_MINUS_SRC_COLOR = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
		DST_COLOR = VK_BLEND_FACTOR_DST_COLOR,
		ONE_MINUS_DST_COLOR = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
		SRC_ALPHA = VK_BLEND_FACTOR_SRC_ALPHA,
		SRC_ONE_MINUS_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		DST_ALPHA = VK_BLEND_FACTOR_DST_ALPHA,
		ONE_MINUS_DST_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
		CONSTANT_COLOR = VK_BLEND_FACTOR_CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA = VK_BLEND_FACTOR_CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
		SRC_ALPHA_SATURATE = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
		SRC1_COLOR = VK_BLEND_FACTOR_SRC1_COLOR,
		ONE_MINUS_SRC1_COLOR = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
		SRC1_ALPHA = VK_BLEND_FACTOR_SRC1_ALPHA,
		ONE_MIUNUS_SRC1_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
	};

	enum class BlendOp
	{
		ADD = VK_BLEND_OP_ADD,
		SUBTRACT = VK_BLEND_OP_SUBTRACT,
		REVERSE_SUBTRACT = VK_BLEND_OP_REVERSE_SUBTRACT,
		MIN = VK_BLEND_OP_MIN,
		MAX = VK_BLEND_OP_MAX
	};

	enum class ColorComponent
	{
		R = VK_COLOR_COMPONENT_R_BIT,
		G = VK_COLOR_COMPONENT_G_BIT,
		B = VK_COLOR_COMPONENT_B_BIT,
		A = VK_COLOR_COMPONENT_A_BIT,
		RGBA = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

public:
	GraphicsPipeline();

	void bind();

	GraphicsPipeline& setSamples(VkSampleCountFlagBits sample_count);
	GraphicsPipeline& setRenderPass(VkRenderPass render_pass);
	GraphicsPipeline& setSubpass(uint32_t subpass);
	GraphicsPipeline& setStage(const PipelineStage& stage);
	GraphicsPipeline& addDynamicState(VkDynamicState dynamic_state);
	GraphicsPipeline& enableTag(EnableTag tag, bool enable = true);
	GraphicsPipeline& setCullMode(CullMode cull_mode);
	GraphicsPipeline& setLineWidth(float width);
	GraphicsPipeline& setPolygonMode(PolygonMode polygon_mode);
	GraphicsPipeline& setFrontFace(FrontFace front_face);
	GraphicsPipeline& setDepthBias(float constant, float slope = 0.0f, float clamp = 0.0f);
	GraphicsPipeline& setDepthCompareOp(CompareOp compare_op);
	GraphicsPipeline& setBlend(BlendFactor src_color, BlendFactor dst_color, BlendFactor src_alpha, BlendFactor dst_alpha);
	GraphicsPipeline& setColorBlendOp(BlendOp blend_op);
	GraphicsPipeline& setAlphaBlendOp(BlendOp blend_op);
	GraphicsPipeline& setColorWriteMask(ColorComponent mask);
	GraphicsPipeline& setShader(const shared<Shader>& shader, const std::vector<Pipeline::Constant>& constants = {});

	void build();

	void create() override;

	const VkGraphicsPipelineCreateInfo& getCreateInfo() const { return ci; }

private:

	VkGraphicsPipelineCreateInfo ci{};
	std::vector<VkDynamicState> dynamic_states{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	VkPipelineColorBlendStateCreateInfo color_blending{};
	VkPipelineViewportStateCreateInfo viewport_info{};
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
	VkPipelineDynamicStateCreateInfo dynamic_state{};
	VkRenderPass render_pass = nullptr;
	uint32_t subpass = 0;
};