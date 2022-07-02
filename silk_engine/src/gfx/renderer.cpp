#include "renderer.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/bezier_mesh.h"
#include "scene/meshes/triangle_mesh.h"
#include "scene/meshes/text_mesh.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/devices/logical_device.h"
#include "gfx/pipeline/default_render_pipeline.h"
#include "pipeline/pipeline_stage.h"
#include "scene/camera/camera.h"
#include "buffers/command_buffer.h"
#include "buffers/indirect_buffer.h"
#include "buffers/uniform_buffer.h"

void Renderer::init()
{
	instance_batches.reserve(Graphics::MAX_INSTANCE_BATCHES); //TODO: remove this limitation some time
	indirect_buffer = makeUnique<IndirectBuffer>(Graphics::MAX_INSTANCE_BATCHES * sizeof(VkDrawIndexedIndirectCommand));
	global_uniform_buffer = makeUnique<UniformBuffer>(sizeof(GlobalUniformData));

	previous_frame_finished = Graphics::logical_device->createFence(true);
	swap_chain_image_available = Graphics::logical_device->createSemaphore();
	render_finished = Graphics::logical_device->createSemaphore();

	command_buffer = new CommandBuffer();

	setRenderPipeline<DefaultRenderPipeline>();
	render_pipeline->init();

	reset();
}

void Renderer::destroy()
{
	Graphics::logical_device->destroyFence(previous_frame_finished);
	Graphics::logical_device->destroySemaphore(swap_chain_image_available);
	Graphics::logical_device->destroySemaphore(render_finished);
	delete command_buffer;
	active = {};
	instances.clear();
	instance_batches.clear();
	indirect_buffer = nullptr;
	global_uniform_buffer = nullptr;
	render_pipeline = nullptr;
}

void Renderer::reset()
{
	for (const auto& instance : instances)
		destroyInstance(*instance);
	instances.clear();	
	lights.fill(Light{});
	active = {};
	active.image = Resources::white_image;
	active.font = Resources::getFont("Arial");
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
	draw(Resources::getGraphicsPipeline("2D"), makeShared<Mesh>(TriangleMesh(x1, y1, x2, y2, x3, y3)), 0, 0, 0, 1, 1, 1);
}

void Renderer::rectangle(float x, float y, float width, float height)
{
	if (active.stroke_weight == 1.0f)
		draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Rectangle"), x, y, 0, width, height, 1);
	else
	{
		float ws = width * active.stroke_weight * 0.5f;
		float hs = height * active.stroke_weight * 0.5f;
		draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Rectangle"), x, y, 0, ws, height, 1, true);
		draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Rectangle"), x, y, 0, width, hs, 1, true);
		draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Rectangle"), x + width - ws, y, 0, ws, height, 1, true);
		draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Rectangle"), x, y + height - hs, 0, width, hs, 1, true);
	}
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
	draw(Resources::getGraphicsPipeline("2D"), makeShared<Mesh>(LineMesh(points, width)), 0, 0, 0, 1, 1, 1);
}

void Renderer::line(float x1, float y1, float x2, float y2, float width)
{
	float l = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	float dx = (x2 - x1) / l;
	float dy = (y2 - y1) / l;
	draw(Resources::getGraphicsPipeline("2D"), Resources::getMesh("Rectangle"), {
			dx * l, dy * l, 0, 0,
			-dy * width, dx * width, 0, 0,
			0, 0, 1, 0,
			x1 + dy * width * 0.5f, y1 - dx * width * 0.5f, 0, 1
		});
}

void Renderer::bezier(float x1, float y1, float px, float py, float x2, float y2, float width)
{
	draw(Resources::getGraphicsPipeline("2D"), makeShared<Mesh>(BezierMesh(x1, y1, px, py, x2, y2, 64u, width)), 0, 0, 0, 1, 1, 1);
}

void Renderer::bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width)
{
	draw(Resources::getGraphicsPipeline("2D"), makeShared<Mesh>(BezierMesh(x1, y1, px1, py1, px2, py2, x2, y2, 64u, width)), 0, 0, 0, 1, 1, 1);
}

void Renderer::text(const std::string& text, float x, float y, float width, float height)
{
	draw(Resources::getGraphicsPipeline("Font"), makeShared<Mesh>(TextMesh(text, 64, active.font)), x, y, 0.0f, width, height, 0.0f, { active.font->getAtlas() });
}

void Renderer::text(const std::string& text, float x, float y, float size)
{
	draw(Resources::getGraphicsPipeline("Font"), makeShared<Mesh>(TextMesh(text, 64, active.font)), x, y, 0.0f, size, size, 0.0f, { active.font->getAtlas() });
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

void Renderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, glm::mat4&& transform, std::vector<shared<Image2D>>&& images, bool stroke)
{
	instances.emplace_back(makeShared<RenderedInstance>(graphics_pipeline, std::move(images)));

	InstanceData data;
	data.transform = std::move(transform);
	data.color = stroke ? active.stroke : active.color;
	if (active.transformed)
		data.transform *= active.transform;
	createInstance(instances.back(), mesh, std::move(data));
}

void Renderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, glm::mat4&& transform, bool stroke)
{
	draw(graphics_pipeline, mesh, std::forward<glm::mat4>(transform), { active.image }, stroke);
}

void Renderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth, std::vector<shared<Image2D>>&& images, bool stroke)
{
	draw(graphics_pipeline, mesh, {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, depth, 0,
		x, y, z, 1
		}, std::forward<std::vector<shared<Image2D>>>(images), stroke);
}

void Renderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth, bool stroke)
{
	draw(graphics_pipeline, mesh, {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, depth, 0,
		x, y, z, 1
		}, stroke);
}

void Renderer::waitForPreviousFrame()
{
	Graphics::logical_device->waitForFences({ previous_frame_finished }, VK_TRUE, UINT64_MAX);
	Graphics::logical_device->resetFences({ previous_frame_finished });
	Graphics::swap_chain->acquireNextImage(swap_chain_image_available);
}

void Renderer::begin(Camera* camera)
{
	updateUniformData(camera);
	updateDrawCommands();

	//Set viewport
	command_buffer->begin();
}

void Renderer::render()
{
	render_pipeline->update();

	PipelineStage stage{};
	for (auto& render_stage : render_pipeline->getRenderStages())
	{
		render_stage.update();

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = Window::getHeight();
		viewport.width = Window::getWidth();
		viewport.height = -(float)Window::getHeight();
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		Graphics::getActiveCommandBuffer().setViewport({ viewport });

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { Window::getWidth(), Window::getHeight() };
		Graphics::getActiveCommandBuffer().setScissor({ scissor });
		
		auto& render_pass = render_stage.getRenderPass();
		render_pass->begin(*render_stage.getFramebuffer());
		for (size_t i = 0; i < render_pass->getSubpassCount(); ++i)
		{
			stage.second = i;
			render_pipeline->renderStage(stage);
			render_pass->nextSubpass();
		}
		render_pass->end();
		++stage.first;
	}
}

void Renderer::end()
{
	CommandBufferSubmitInfo submit_info{};
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit_info.wait_stages = &wait_stage;
	submit_info.wait_semaphores = { swap_chain_image_available };
	submit_info.signal_semaphores = { render_finished };
	submit_info.fence = previous_frame_finished;
	command_buffer->submit(submit_info);
	Graphics::vulkanAssert(Graphics::swap_chain->present(render_finished));

	Graphics::stats.instance_batches += instance_batches.size();
	for (const auto& instance_batch : instance_batches)
	{
		Graphics::stats.instances += instance_batch.instances.size();
		Graphics::stats.vertices += instance_batch.mesh->getVertexCount() * instance_batch.instances.size();
		Graphics::stats.indices += instance_batch.mesh->getIndexCount() * instance_batch.instances.size();
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

void Renderer::createInstance(const shared<RenderedInstance>& instance, const shared<Mesh>& mesh, const InstanceData& instance_data)
{
	if (!instance->material.get()) 
		instance->material = Resources::getGraphicsPipeline("Lit 3D");

	bool need_new_instance_batch = true;
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		auto& instance_batch = instance_batches[i];
		if (instance_batch == *instance && instance_batch.mesh == mesh && instance_batch.instance_data.size() < Graphics::MAX_INSTANCES && instance_batch.instance_images.available() >= instance->images.size())
		{
			instance_batch.needs_update = true;
			instance_batch.instance_data.push_back(instance_data);
			instance_batch.instances.push_back(instance);

			instance->instance_batch_index = i;
			instance->instance_data_index = instance_batch.instance_data.size() - 1;

			need_new_instance_batch = false;
			break;
		}
	}

	if (need_new_instance_batch)
		addInstanceBatch(instance, mesh, instance_data);

	if (instance->images.size())
	{
		uint32_t image_index = instance_batches[instance->instance_batch_index].instance_images.add(instance->images);
		SK_ASSERT(image_index != UINT32_MAX, "Instance has too much images");
		instance_batches[instance->instance_batch_index].instance_data[instance->instance_data_index].image_index = image_index;
	}
}

void Renderer::updateInstance(RenderedInstance& instance, const InstanceData& instance_data)
{
	instance_batches[instance.instance_batch_index].needs_update = true;
	instance_batches[instance.instance_batch_index].instance_data[instance.instance_data_index] = instance_data;
}

void Renderer::addInstanceBatch(const shared<RenderedInstance>& instance, const shared<Mesh>& mesh, const InstanceData& instance_data)
{
	instance_batches.emplace_back(mesh, instance);
	auto& new_batch = instance_batches.back();

	new_batch.instance_images.add({ Resources::white_image });

	new_batch.instance_data.emplace_back(instance_data);
	new_batch.instances.emplace_back(instance);

	new_batch.instance_buffer = makeShared<VertexBuffer>(nullptr, Graphics::MAX_INSTANCES * sizeof(InstanceData), VMA_MEMORY_USAGE_CPU_TO_GPU);

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

void Renderer::updateUniformData(Camera* camera)
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
}

void Renderer::updateDrawCommands()
{
	bool any_needs_update = false;
	static std::vector<VkDrawIndexedIndirectCommand> draw_commands;
	draw_commands.resize(instance_batches.size());
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		if (instance_batches[i].needs_update)
		{
			instance_batches[i].instance_buffer->setData(instance_batches[i].instance_data.data(), instance_batches[i].instance_data.size() * sizeof(InstanceData));
			instance_batches[i].needs_update = false;
			VkDrawIndexedIndirectCommand draw_command{};
			draw_command.instanceCount = instance_batches[i].instance_data.size();
			draw_command.indexCount = instance_batches[i].mesh->getIndexCount();
			draw_commands[i] = std::move(draw_command);
			any_needs_update = true;
		}
	}
	if (any_needs_update)
		indirect_buffer->setData(draw_commands.data(), draw_commands.size() * sizeof(VkDrawIndexedIndirectCommand));
}