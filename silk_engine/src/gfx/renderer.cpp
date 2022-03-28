#include "renderer.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "scene/resources.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/bezier_mesh.h"
#include "scene/meshes/triangle_mesh.h"
#include "gfx/buffers/command_buffer.h"
#include <glm/gtc/matrix_transform.hpp>

void Renderer::init()
{
	instance_batches.reserve(Graphics::MAX_INSTANCE_BATCHES); //TODO: remove this limitation some time
	indirect_buffer = makeUnique<IndirectBuffer>(Graphics::MAX_INSTANCE_BATCHES * sizeof(vk::DrawIndexedIndirectCommand));
	global_uniform_buffer = makeUnique<UniformBuffer>(sizeof(GlobalUniformData));
	lights.fill(Light{});
	active.image = Resources::white_image;
}

void Renderer::cleanup()
{
	active.image = nullptr;
	instances.clear();
	instance_batches.clear();
	indirect_buffer = nullptr;
	global_uniform_buffer = nullptr;
}

void Renderer::reset()
{
	for (const auto& instance : instances)
		destroyInstance(*instance);
	instances.clear();
	active.image = Resources::white_image;
	active.transform = glm::mat4(1);
	active.color = glm::vec4(1);
}

void Renderer::triangle(float x, float y, float width, float height)
{
	draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Triangle"), x, y, 0.0f, width, height, 1);
}

void Renderer::triangle(float x, float y, float size)
{
	triangle(x, y, size, size);
}

void Renderer::triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	draw(Resources::getGraphicsPipeline("2D"), makeShared<TriangleMesh>(x1, y1, x2, y2, x3, y3), 0, 0, 0, 1, 1, 1);
}

void Renderer::rectangle(float x, float y, float width, float height)
{
	draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Rectangle"), x, y, 0, width, height, 1);
}

void Renderer::square(float x, float y, float size)
{
	rectangle(x, y, size, size);
}

void Renderer::ellipse(float x, float y, float width, float height)
{
	draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Circle"), x, y, 0, width, height, 1);
}

void Renderer::circle(float x, float y, float radius)
{
	ellipse(x, y, radius, radius);
}

void Renderer::line(const std::vector<glm::vec2>& points, float width)
{
	draw(Resources::getGraphicsPipeline("2D"), makeShared<LineMesh>(points, width), 0, 0, 0, 1, 1, 1);
}

void Renderer::line(float x1, float y1, float x2, float y2)
{
	//TODO: It might be faster to not generate line every time and just rotate pregenerated line
	line({ { x1, y1 }, { x2, y2 } });
}

void Renderer::bezier(float x1, float y1, float px, float py, float x2, float y2, float width)
{
	draw(Resources::getGraphicsPipeline("2D"), makeShared<BezierMesh>(x1, y1, px, py, x2, y2, 64u, width), 0, 0, 0, 1, 1, 1);
}

void Renderer::bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width)
{
	draw(Resources::getGraphicsPipeline("2D"), makeShared<BezierMesh>(x1, y1, px1, py1, px2, py2, x2, y2, 64u, width), 0, 0, 0, 1, 1, 1);
}

void Renderer::tetrahedron(float x, float y, float z, float size)
{
	draw(Resources::getGraphicsPipeline("3D"), Resources::getMesh("Tetrahedron"), x, y, z, size, size, size);
}

void Renderer::cube(float x, float y, float z, float size)
{
	draw(Resources::getGraphicsPipeline("3D"), Resources::getMesh("Cube"), x, y, z, size, size, size);
}

void Renderer::cuboid(float x, float y, float z, float width, float height, float depth)
{
	draw(Resources::getGraphicsPipeline("3D"), Resources::getMesh("Cube"), x, y, z, width, height, depth);
}

void Renderer::sphere(float x, float y, float z, float radius)
{
	draw(Resources::getGraphicsPipeline("3D"), Resources::getMesh("Sphere"), x, y, z, radius, radius, radius);
}

void Renderer::ellipsoid(float x, float y, float z, float width, float height, float depth)
{
	draw(Resources::getGraphicsPipeline("3D"), Resources::getMesh("Sphere"), x, y, z, width, height, depth);
}

void Renderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth)
{
	shared<RenderedInstance> instance = makeShared<RenderedInstance>();
	instance->mesh = mesh;
	instance->material = graphics_pipeline;
	instance->images = { active.image };
	instances.emplace_back(instance);

	InstanceData data{};
	data.transform = glm::mat4
	(
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, depth, 0,
		x, y, z, 1
	) * active.transform;
	data.color = active.color;
	createInstance(instance, std::move(data));
}

void Renderer::update(Camera* camera)
{
	GlobalUniformData global_uniform_data{};
	if (camera)
	{
		global_uniform_data.projection_view = camera->projection_view;
		global_uniform_data.projection = camera->projection;
		global_uniform_data.view = camera->view;
		global_uniform_data.camera_position = camera->position;
		global_uniform_data.camera_direction = camera->direction;
	}
	if (false /*Is main_camera 2D?*/)
	{
		//TODO: Support orthographic 2D camera  
	}
	else
	{
		global_uniform_data.projection_view2D = glm::ortho(0.0f, (float)Window::getWidth(), 0.0f, (float)Window::getHeight(), 0.0f, 1.0f);
	}
	global_uniform_data.delta_time = Time::dt;
	global_uniform_data.time = Time::runtime;
	global_uniform_data.frame = Time::frame;
	global_uniform_data.resolution = glm::uvec2(Window::getWidth(), Window::getHeight());
	global_uniform_data.lights = lights;

	global_uniform_buffer->setData(&global_uniform_data, sizeof(GlobalUniformData));

	//Drawing	
	bool any_needs_update = false;
	static std::vector<vk::DrawIndexedIndirectCommand> draw_commands;
	draw_commands.resize(instance_batches.size());
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		if (instance_batches[i].needs_update)
		{
			instance_batches[i].instance_buffer->setData(instance_batches[i].instance_data.data(), instance_batches[i].instance_data.size() * sizeof(InstanceData));
			instance_batches[i].needs_update = false;
			vk::DrawIndexedIndirectCommand draw_command{};
			draw_command.instanceCount = instance_batches[i].instance_data.size();
			draw_command.indexCount = instance_batches[i].instance->mesh->indices.size();
			draw_commands[i] = std::move(draw_command);
			any_needs_update = true;
		}
	}
	if (any_needs_update)
		indirect_buffer->setData(draw_commands.data(), draw_commands.size() * sizeof(vk::DrawIndexedIndirectCommand));

	//Draw instances
	Graphics::swap_chain->getRenderPass()->begin(*Graphics::swap_chain->getActiveFramebuffer(), vk::SubpassContents::eInline);
	size_t draw_index = 0;
	for (auto& instance_batch : instance_batches)
	{
		const auto& shader = instance_batch.instance->material->getShader();
		if (auto global_uniform = shader->getIfExists("GlobalUniform"))
			instance_batch.descriptor_sets[global_uniform->set].setBufferInfo(global_uniform->write_index, { *global_uniform_buffer }); //TODO: Global data doesn't have to update for each batch

		if (auto images = shader->getIfExists("images"))
			instance_batch.instance_images.updateDescriptorSet(instance_batch.descriptor_sets[images->set], images->write_index);

		instance_batch.bind();
		Graphics::getActiveCommandBuffer().drawIndexedIndirect(*indirect_buffer, draw_index * sizeof(vk::DrawIndexedIndirectCommand), 1, sizeof(vk::DrawIndexedIndirectCommand));
		++draw_index;
	}
	//End draw
	Graphics::swap_chain->getRenderPass()->end();

	Graphics::stats.instance_batches += instance_batches.size();
	for (const auto& instance_batch : instance_batches)
	{
		Graphics::stats.instances += instance_batch.instances.size();
		Graphics::stats.vertices += instance_batch.instance->mesh->vertexCount() * instance_batch.instances.size();
		Graphics::stats.indices += instance_batch.instance->mesh->indices.size() * instance_batch.instances.size();
	}
}

Light* Renderer::addLight(const Light& light)
{
	for (auto& l : lights)
	{
		if (l.color == glm::vec3(0))
		{
			l = light;
			return &l;
		}
	}
	return nullptr;
}

void Renderer::createInstance(const shared<RenderedInstance>& instance, const InstanceData& instance_data)
{
	if (!instance->mesh->vertex_array.get()) instance->mesh->createVertexArray();
	//if (!instance->mesh->hasAABB()) instance->mesh->calculateAABB(); //TEMP for now
	if (!instance->material.get()) instance->material = Resources::getGraphicsPipeline("Lit 3D");

	bool need_new_instance_batch = true;
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		auto& instance_batch = instance_batches[i];
		if (instance_batch == *instance && instance_batch.instance_data.size() < Graphics::MAX_INSTANCES && instance_batch.instance_images.available() >= instance->images.size())
		{
			instance_batch.needs_update = true;
			instance_batch.instance_data.emplace_back(std::move(instance_data));
			instance_batch.instances.emplace_back(instance);

			instance->instance_batch_index = i;
			instance->instance_data_index = instance_batch.instance_data.size() - 1;

			need_new_instance_batch = false;
			break;
		}
	}

	if (need_new_instance_batch)
		addInstanceBatch(instance, instance_data);

	if (instance->images.size())
	{
		uint32_t image_index = instance_batches[instance->instance_batch_index].instance_images.add(instance->images);
		SK_ASSERT(image_index != UINT32_MAX, "Instance has too much images");
		instance_batches[instance->instance_batch_index].instance_data[instance->instance_data_index].image_index = image_index;
	}
}

void Renderer::updateInstance(RenderedInstance& instance, const InstanceData& instance_data)
{
	auto& instance_batch = instance_batches[instance.instance_batch_index];
	instance_batch.needs_update = true;
	instance_batch.instance_data[instance.instance_data_index] = instance_data;
}

void Renderer::addInstanceBatch(const shared<RenderedInstance>& instance, const InstanceData& instance_data)
{
	instance_batches.emplace_back(instance);
	auto& new_batch = instance_batches.back();
	new_batch.instance_data.reserve(Graphics::MAX_INSTANCES);
	new_batch.instances.reserve(Graphics::MAX_INSTANCES);

	new_batch.instance_images.add({ Resources::white_image });

	new_batch.instance_data.emplace_back(std::move(instance_data));
	new_batch.instances.emplace_back(instance);

	new_batch.instance_buffer = makeShared<VertexBuffer>(new_batch.instance_data.data(), Graphics::MAX_INSTANCES * sizeof(InstanceData), VMA_MEMORY_USAGE_CPU_TO_GPU);

	for (auto&& [set, descriptor_set] : new_batch.instance->material->getShader()->getDescriptorSets())
		new_batch.descriptor_sets[set] = *descriptor_set;


	instance->instance_batch_index = instance_batches.size() - 1;
	instance->instance_data_index = instance_batches.back().instance_data.size() - 1;
}

void Renderer::destroyInstance(const RenderedInstance& instance)
{
	auto& instance_batch = instance_batches[instance.instance_batch_index];

	instance_batch.instances.back()->instance_data_index = instance.instance_data_index;

	if (instance_batch.instance_data.size() - 1 == 0)
	{
		if (instance.instance_batch_index != instance_batches.size() - 1)
		{
			InstanceBatch* last_batch = &instance_batch;
			instance_batches.back().needs_update = true; //Instance batches data got swapped around so this forces to also update instance_batch draw commands buffer
			std::swap(instance_batch, instance_batches.back());
			for (size_t j = 0; j < last_batch->instances.size(); ++j)
				last_batch->instances[j]->instance_batch_index = instance_batches.back().instance->instance_batch_index;
		}
		instance_batches.pop_back();
		return;
	}

	instance_batch.instance_images.remove(instance_batch.instance_data[instance.instance_data_index].image_index, instance.images.size());

	instance_batch.needs_update = true;

	std::swap(instance_batch.instance_data[instance.instance_data_index], instance_batch.instance_data.back());
	instance_batch.instance_data.pop_back();

	std::swap(instance_batch.instances[instance.instance_data_index], instance_batch.instances.back());
	instance_batch.instances.pop_back();
}