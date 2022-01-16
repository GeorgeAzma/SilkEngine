#pragma once

#include "vulkan_engine.h"
#include "core/entry_point.h"

class SandboxApp : public Application
{
public:
    SandboxApp(ApplicationCommandLineArgs args);
    void onUpdate();
    ~SandboxApp();

private:
	Scene scene;
	std::shared_ptr<Entity> camera;
	std::shared_ptr<UniformBuffer> uniform_buffer = nullptr;
	std::shared_ptr<VertexBuffer> vertex_buffer = nullptr;
	std::shared_ptr<VertexBuffer> vertex_buffer2 = nullptr;
	std::shared_ptr<VertexArray> vertex_array = nullptr;
	std::shared_ptr<IndexBuffer> index_buffer = nullptr;
	std::shared_ptr<Image> image = nullptr;
	std::shared_ptr<DescriptorSetLayout> descriptor_set_layout = nullptr;
	std::shared_ptr<DescriptorSet> descriptor_set = nullptr;
	std::shared_ptr<GraphicsPipeline> graphics_pipeline = nullptr;
	const std::vector<Vertex> vertices =
	{
		{{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
	};
	const std::vector<uint32_t> indices =
	{
		0, 1, 2, 2, 3, 0
	};

	struct Transforms
	{
		glm::mat4 projection_view;
	};
};