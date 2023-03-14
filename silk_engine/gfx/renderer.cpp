#include "renderer.h"
#include "gfx/graphics.h"
#include "scene/instance.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/bezier_mesh.h"
#include "scene/meshes/triangle_mesh.h"
#include "scene/meshes/text_mesh.h"
#include "gfx/devices/logical_device.h"
#include "gfx/pipeline/default_render_pipeline.h"
#include "pipeline/pipeline_stage.h"
#include "scene/camera/camera.h"
#include "buffers/command_buffer.h"
#include "gfx/fence.h"

unique<RenderPipeline> Renderer::render_pipeline = nullptr;
std::vector<InstanceBatch> Renderer::instance_batches{};
std::vector<shared<RenderedInstance>> Renderer::instances{};
unique<Buffer> Renderer::indirect_buffer = nullptr;
unique<Buffer> Renderer::global_uniform_buffer = nullptr;
std::array<Light, Renderer::MAX_LIGHTS> Renderer::lights{};
Fence* Renderer::previous_frame_finished = nullptr;
VkSemaphore Renderer::swap_chain_image_available = nullptr;
VkSemaphore Renderer::render_finished = nullptr;

void Renderer::init()
{
	instance_batches.reserve(MAX_INSTANCE_BATCHES); //TODO: remove this limitation some time
	indirect_buffer = makeUnique<Buffer>(MAX_INSTANCE_BATCHES * sizeof(VkDrawIndexedIndirectCommand), Buffer::INDIRECT | Buffer::TRANSFER_DST, Allocation::Props{ Allocation::MAPPED | Allocation::RANDOM_ACCESS, Allocation::Device::CPU });
	global_uniform_buffer = makeUnique<Buffer>(sizeof(GlobalUniformData), Buffer::UNIFORM | Buffer::TRANSFER_DST, Allocation::Props{ Allocation::MAPPED | Allocation::SEQUENTIAL_WRITE });

	previous_frame_finished = new Fence(true);
	swap_chain_image_available = Graphics::logical_device->createSemaphore();
	render_finished = Graphics::logical_device->createSemaphore();

	setRenderPipeline<DefaultRenderPipeline>();
	render_pipeline->init();
	onResize();

	reset();
}

void Renderer::destroy()
{
	delete previous_frame_finished;
	Graphics::logical_device->destroySemaphore(swap_chain_image_available);
	Graphics::logical_device->destroySemaphore(render_finished);
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
	active = {};
	active.image = Resources::white_image;
	active.font = Resources::get<Font>("Arial");
}

void Renderer::wait()
{
	previous_frame_finished->wait();
	previous_frame_finished->reset();
}

void Renderer::onResize()
{
	if (Window::getActive().isMinimized())
		return;
	DebugTimer t("Renderer::onResize");
	Window::getActive().recreate();
	for (auto& render_stage : render_pipeline->getRenderStages())
		render_stage.onResize(Window::getActive().getSwapChain());
}

void Renderer::triangle(float x, float y, float width, float height)
{
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Triangle"), x, y, width, height);
}

void Renderer::triangle(float x, float y, float size)
{
	triangle(x, y, size, size);
}

void Renderer::triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	draw(Resources::get<GraphicsPipeline>("2D"), makeShared<Mesh>(TriangleMesh(x1, y1, x2, y2, x3, y3)), 0, 0, 1, 1);
}

void Renderer::rectangle(float x, float y, float width, float height)
{
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Rectangle"), x, y, width, height);
}

void Renderer::roundedRectangle(float x, float y, float width, float height)
{
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Rounded Rectangle"), x, y, width, height);
}

void Renderer::square(float x, float y, float size)
{
	rectangle(x, y, size, size);
}

void Renderer::roundedSquare(float x, float y, float size)
{
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Rounded Rectangle"), x, y, size, size);
}

void Renderer::ellipse(float x, float y, float width, float height)
{
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Circle"), x, y, width, height);
}

void Renderer::ellipseOutline(float x, float y, float width, float height)
{
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Circle Outline"), x, y, width, height);
}

void Renderer::circle(float x, float y, float radius)
{
	ellipse(x, y, radius, radius);
}

void Renderer::circleOutline(float x, float y, float radius)
{
	ellipseOutline(x, y, radius, radius);
}

void Renderer::line(const std::vector<vec2>& points, float width)
{
	draw(Resources::get<GraphicsPipeline>("2D"), makeShared<Mesh>(LineMesh(points, width)), 0, 0, 1, 1);
}

void Renderer::line(float x1, float y1, float x2, float y2, float width)
{
	float l = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	float dx = (x2 - x1) / l;
	float dy = (y2 - y1) / l;
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Rectangle"), {
			dx * l, dy * l, 0, 0,
			-dy * width, dx * width, 0, 0,
			0, 0, 1, 0,
			x1 + dy * width * 0.5f, y1 - dx * width * 0.5f, 1, 1
		});
}

void Renderer::bezier(float x1, float y1, float px, float py, float x2, float y2, float width)
{
	draw(Resources::get<GraphicsPipeline>("2D"), makeShared<Mesh>(BezierMesh(x1, y1, px, py, x2, y2, 64u, width)), 0, 0, 1, 1);
}

void Renderer::bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width)
{
	draw(Resources::get<GraphicsPipeline>("2D"), makeShared<Mesh>(BezierMesh(x1, y1, px1, py1, px2, py2, x2, y2, 64u, width)), 0, 0, 1, 1);
}

void Renderer::text(const std::string& text, float x, float y, float width, float height)
{
	draw(Resources::get<GraphicsPipeline>("Font"), makeShared<Mesh>(TextMesh(text, 64, active.font)), x, y, width, height, { active.font->getAtlas() });
}

void Renderer::text(const std::string& text, float x, float y, float size)
{
	if (text.empty())
		return;
	draw(Resources::get<GraphicsPipeline>("Font"), makeShared<Mesh>(TextMesh(text, 32, active.font)), x, y, size, size, { active.font->getAtlas() });
}

void Renderer::tetrahedron(float x, float y, float z, float size)
{
	draw(Resources::get<GraphicsPipeline>("3D"), Resources::get<Mesh>("Tetrahedron"), x, y, z, size, size, size);
}

void Renderer::cube(float x, float y, float z, float size)
{
	draw(Resources::get<GraphicsPipeline>("3D"), Resources::get<Mesh>("Cube"), x, y, z, size, size, size);
}

void Renderer::cuboid(float x, float y, float z, float width, float height, float depth)
{
	draw(Resources::get<GraphicsPipeline>("3D"), Resources::get<Mesh>("Cube"), x, y, z, width, height, depth);
}

void Renderer::sphere(float x, float y, float z, float radius)
{
	draw(Resources::get<GraphicsPipeline>("3D"), Resources::get<Mesh>("Sphere"), x, y, z, radius, radius, radius);
}

void Renderer::ellipsoid(float x, float y, float z, float width, float height, float depth)
{
	draw(Resources::get<GraphicsPipeline>("3D"), Resources::get<Mesh>("Sphere"), x, y, z, width, height, depth);
}

void Renderer::image(const shared<Image>& image, float x, float y, float width, float height)
{
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Rectangle"), x, y, width, height, { image });
}

void Renderer::image(const shared<Image>& image, float x, float y, float size)
{
	draw(Resources::get<GraphicsPipeline>("2D"), Resources::get<Mesh>("Rectangle"), x, y, size, size, { image });
}

void Renderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, const mat4& transform, const std::vector<shared<Image>>& images)
{
	instances.emplace_back(makeShared<RenderedInstance>(graphics_pipeline, images));

	InstanceData data;
	data.transform = transform;
	data.color = active.color;
	if (active.transformed)
		data.transform *= active.transform;
	createInstance(instances.back(), mesh, std::move(data));
}

void Renderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth, const std::vector<shared<Image>>& images)
{
	draw(graphics_pipeline, mesh, {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, depth, 0,
		x, y, z, 1
		}, images.size() ? images : (active.image.get() ? std::vector<shared<Image>>{ active.image } : std::vector<shared<Image>>{}));
}

void Renderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float width, float height, const std::vector<shared<Image>>& images)
{
	draw(graphics_pipeline, mesh, {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, 1, 0,
		x, y, active.depth, 1
		}, images.size() ? images : (active.image.get() ? std::vector<shared<Image>>{ active.image } : std::vector<shared<Image>>{}));
	active.depth -= 1e-10;
}

void Renderer::render(Camera* camera)
{
	stats = {};
	Graphics::update();
	updateUniformData(camera);
	updateDrawCommands();

	if (!Window::getActive().getSwapChain().acquireNextImage(Renderer::swap_chain_image_available))
	{
		SK_ERROR("Unexpected, window should already be updated |||||\n\n\n\n\n\n\n\n\n\n\n\n\n||||||||||||||||||||||");
		onResize();
	}

	render_pipeline->update(); 

	Graphics::submit([&](CommandBuffer& cb)
		{
			PipelineStage stage{};
			for (auto& render_stage : render_pipeline->getRenderStages())
			{
				float width = render_stage.getFramebuffer()->getWidth();
				float height = render_stage.getFramebuffer()->getHeight();
				
				VkViewport viewport = {};
				viewport.x = 0.0f;
				viewport.y = height;
				viewport.width = width;
				viewport.height = -height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				cb.setViewport({ viewport });
				
				VkRect2D scissor = {};
				scissor.offset = { 0, 0 };
				scissor.extent = { (uint32_t)width, (uint32_t)height };
				cb.setScissor({ scissor });
				
				auto& render_pass = render_stage.getRenderPass();
				render_pass->begin(*render_stage.getFramebuffer());
				for (size_t i = 0; i < render_pass->getSubpassCount(); ++i)
				{
					stage.subpass = i;
					render_pipeline->renderStage(stage);
					if (i < render_pass->getSubpassCount() - 1)
						render_pass->nextSubpass();
				}
				render_pass->end();
				++stage.render_pass;
			}
		});

	CommandBuffer::SubmitInfo submit_info{};
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit_info.wait_stages = &wait_stage;
	submit_info.wait_semaphores = { swap_chain_image_available };
	submit_info.signal_semaphores = { render_finished };
	submit_info.fence = previous_frame_finished;
	Graphics::execute(submit_info);
	if (!Window::getActive().getSwapChain().present(render_finished))
		onResize();

	stats.instance_batches += instance_batches.size();
	for (const auto& instance_batch : instance_batches)
	{
		stats.instances += instance_batch.instances.size();
		stats.vertices += instance_batch.mesh->getVertexCount() * instance_batch.instances.size();
		stats.indices += instance_batch.mesh->getIndexCount() * instance_batch.instances.size();
	}
}

Light* Renderer::addLight(const Light& light)
{
	for (auto& l : lights)
	{
		if (l.color == vec3(0))
		{
			l = light;
			return &l;
		}
	}

	lights.back() = light;
	return &lights.back();
}

void Renderer::createInstance(const shared<RenderedInstance>& instance, const shared<Mesh>& mesh, const InstanceData& instance_data)
{
	bool need_new_instance_batch = true;
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		auto& instance_batch = instance_batches[i];
		if (instance_batch == *instance && instance_batch.mesh == mesh && instance_batch.instance_data.size() < MAX_INSTANCES && instance_batch.instance_images.available() >= instance->images.size())
		{
			instance_batch.needs_update = true;
			instance_batch.instance_data.emplace_back(instance_data);
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
		SK_VERIFY(image_index != UINT32_MAX, "Instance has too much images");
		instance_batches[instance->instance_batch_index].instance_data[instance->instance_data_index].image_index = image_index;
	}
}

void Renderer::updateInstance(RenderedInstance& instance, const InstanceData& instance_data)
{
	instance_batches[instance.instance_batch_index].needs_update = true;
	instance_batches[instance.instance_batch_index].instance_data[instance.instance_data_index] = instance_data;
}

InstanceBatch& Renderer::getInstanceBatch(size_t index)
{
	return instance_batches[index];
}

std::vector<InstanceBatch>& Renderer::getInstanceBatches()
{
	return instance_batches;
}

const unique<Buffer>& Renderer::getIndirectBuffer()
{
	return indirect_buffer;
}

const unique<Buffer>& Renderer::getGlobalUniformBuffer()
{
	return global_uniform_buffer;
}

void Renderer::addInstanceBatch(const shared<RenderedInstance>& instance, const shared<Mesh>& mesh, const InstanceData& instance_data)
{
	instance_batches.emplace_back(mesh, instance);
	auto& new_batch = instance_batches.back();

	new_batch.instance_images.add({ Resources::white_image });

	new_batch.instance_data.emplace_back(instance_data);
	new_batch.instances.emplace_back(instance);

	new_batch.instance_buffer = makeShared<VertexBuffer>(nullptr, sizeof(InstanceData), MAX_INSTANCES, true);

	for (auto&& [set, descriptor_set] : new_batch.instance->pipeline->getShader()->getDescriptorSets())
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
			//instance_batches.back().needs_update = true; //Instance batches data got swapped around so this forces to also update instance_batch draw commands buffer
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
	global_uniform_data.projection_view2D = math::ortho(0.0f, (float)Window::getActive().getWidth(), 0.0f, (float)Window::getActive().getHeight(), 0.0f, 1.0f);
	global_uniform_data.delta_time = Time::dt;
	global_uniform_data.time = Time::runtime;
	global_uniform_data.frame = Time::frame;
	global_uniform_data.resolution = uvec2(Window::getActive().getWidth(), Window::getActive().getHeight());
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