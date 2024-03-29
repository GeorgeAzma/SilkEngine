#include "debug_renderer.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/descriptors/descriptor_set.h"
#include "silk_engine/scene/meshes/mesh.h"
#include "silk_engine/scene/meshes/bezier_mesh.h"
#include "silk_engine/scene/meshes/line_mesh.h"
#include "silk_engine/scene/meshes/text_mesh.h"
#include "silk_engine/scene/meshes/triangle_mesh.h"
#include "silk_engine/scene/meshes/circle_mesh.h"
#include "silk_engine/scene/meshes/circle_outline_mesh.h"
#include "silk_engine/scene/meshes/rectangle_mesh.h"
#include "silk_engine/scene/meshes/rounded_rectangle_mesh.h"
#include "silk_engine/scene/meshes/quad_mesh.h"
#include "silk_engine/scene/meshes/cube_mesh.h"
#include "silk_engine/scene/meshes/sphere_mesh.h"
#include "silk_engine/scene/model.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/gfx/images/image.h"
#include "silk_engine/gfx/ui/font.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/pipeline/render_graph/render_graph.h"
#include "silk_engine/gfx/pipeline/material.h"
#include "silk_engine/gfx/pipeline/render_pass.h"

shared<Buffer> DebugRenderer::global_uniform_buffer = nullptr;
std::array<Light, DebugRenderer::MAX_LIGHTS> DebugRenderer::lights{};
shared<Image> DebugRenderer::white_image = nullptr;
shared<GraphicsPipeline> DebugRenderer::graphics_pipeline_2D = nullptr;
shared<GraphicsPipeline> DebugRenderer::graphics_pipeline_3D = nullptr;

void DebugRenderer::InstancedRendererBase::InstanceGroup::bind()
{
	material->set("GlobalUniform", *global_uniform_buffer);
	material->set("images", instance_images.getDescriptorImageInfos());
	material->bind();
	mesh->bind();
	instance_buffer->bindVertex(1);
}

void DebugRenderer::InstancedRendererBase::init()
{
	indirect_buffer = makeShared<Buffer>(256 * sizeof(VkDrawIndexedIndirectCommand), BufferUsage::INDIRECT, Allocation::Props{ Allocation::MAPPED | Allocation::RANDOM_ACCESS });
}

void DebugRenderer::InstancedRendererBase::update()
{
	bool any_needs_update = false;
	draw_commands.resize(instance_batches.size());

	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		auto& instance_batch = instance_batches[i];
		if (instance_batch.needs_update && instance_batch.instance_count)
		{
			instance_batch.needs_update = false;
			if (!any_needs_update)
				RenderContext::getLogicalDevice().wait();
			any_needs_update = true;
			draw_commands[i].instanceCount = instance_batch.instance_count;
			draw_commands[i].firstIndex = instance_batch.first_index;
			draw_commands[i].indexCount = instance_batch.index_count;
			if (instance_batch.instance_data.size() > instance_batch.instance_buffer->getSize())
				instance_batch.instance_buffer->resize(instance_batch.instance_data.size() * 2);
			instance_batch.instance_buffer->setData(instance_batch.instance_data.data(), instance_batch.instance_data.size());
		}
	}

	if (any_needs_update && !indirect_buffer->setData(draw_commands.data(), draw_commands.size() * sizeof(VkDrawIndexedIndirectCommand)))
		indirect_buffer->resize(indirect_buffer->getSize() * 2);
}

void DebugRenderer::InstancedRendererBase::render()
{
	uint32_t draw_index = 0;
	for (auto& instance_batch : instance_batches)
	{
		instance_batch.bind();
		indirect_buffer->drawIndexedIndirect(draw_index);
		++draw_index;
	}
}

DebugRenderer::Renderable DebugRenderer::InstancedRendererBase::createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const void* instance_data, size_t instance_data_size, size_t image_index_offset, const shared<GraphicsPipeline>& pipeline, const std::vector<shared<Image>>& images)
{
	size_t hash = size_t(mesh.get()) ^ size_t(pipeline.get()) ^ size_t(first_index) ^ (size_t(index_count) << 32);
	size_t instance_batch_index = instance_batches.size();
	size_t instance_data_index = 0;
	bool need_new_instance_batch = true;
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		auto& instance_batch = instance_batches[i];
		if (instance_batch.hash == hash && instance_batch.instance_images.available() >= images.size())
		{
			instance_batch_index = i;
			instance_data_index = instance_batch.instance_data.size();
			instance_batch.addData(instance_data);
			break;
		}
	}

	if (instance_batch_index == instance_batches.size())
	{
		instance_batches.emplace_back();
		auto& new_batch = instance_batches.back();
		new_batch.mesh = mesh;
		new_batch.first_index = first_index;
		new_batch.index_count = index_count;
		new_batch.data_size = instance_data_size;
		new_batch.addData(instance_data);
		new_batch.material = makeShared<Material>(pipeline);
		new_batch.instance_buffer = makeShared<Buffer>(65536, BufferUsage::VERTEX, Allocation::Props{ Allocation::SEQUENTIAL_WRITE | Allocation::MAPPED });
		new_batch.instance_images.add({ white_image });
		new_batch.hash = hash;
	}

	uint32_t image_index = 0;
	if (image_index_offset != std::numeric_limits<size_t>::max() && images.size())
	{
		image_index = instance_batches[instance_batch_index].instance_images.add(images);
		SK_VERIFY(image_index != UINT32_MAX, "Instance has too much images");
		memcpy(&instance_batches[instance_batch_index].instance_data[instance_data_index + image_index_offset], &image_index, sizeof(uint32_t));
	}

	return Renderable(instance_data_index, instance_batch_index, image_index, images.size());
}

shared<DebugRenderer::Renderable> DebugRenderer::InstancedRenderer::createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const void* instance_data, size_t instance_data_size, size_t image_index_offset, const shared<GraphicsPipeline>& pipeline, const std::vector<shared<Image>>& images)
{
	Renderable instance = DebugRenderer::InstancedRendererBase::createInstance(mesh, first_index, index_count, instance_data, instance_data_size, image_index_offset, pipeline, images);
	if (instance.batch_index >= instances.size())
		instances.emplace_back();
	auto& instance_vec = instances[instance.batch_index];
	instance_vec.emplace_back(makeShared<Renderable>(std::move(instance)));
	return instance_vec.back();
}

void DebugRenderer::InstancedRenderer::destroyInstance(const Renderable& instance)
{
	auto& instance_batch = instance_batches[instance.batch_index];
	if (instance_batch.instance_data.empty())
		return;

	if (instance_batch.instance_data.size() == 1)
	{
		if (instance.batch_index < instance_batches.size() - 1)
		{
			std::swap(instance_batch, instance_batches.back());
			for (auto& i : instances.back())
				i->batch_index = instance.batch_index;
		}
		instance_batches.pop_back();
		return;
	}

	instance_batch.instance_images.remove(instance.image_index, instance.image_count);
	std::swap_ranges(instance_batch.instance_data.begin() + instance.data_offset, instance_batch.instance_data.begin() + instance.data_offset + instance_batch.data_size, instance_batch.instance_data.end() - instance_batch.data_size);
	instance_batch.instance_data.erase(instance_batch.instance_data.end() - instance_batch.data_size);
	--instance_batch.instance_count;
	instance_batch.needs_update = true;

	instances[instance.batch_index].back()->data_offset = instance.data_offset;
	std::swap(instances[instance.batch_index][instance.data_offset], instances[instance.batch_index].back());
	instances[instance.batch_index].pop_back();
}

void DebugRenderer::ImmediateInstancedRenderer::createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const void* instance_data, size_t instance_data_size, size_t image_index_offset, const shared<GraphicsPipeline>& pipeline, const std::vector<shared<Image>>& images)
{
	Renderable instance = DebugRenderer::InstancedRendererBase::createInstance(mesh, first_index, index_count, instance_data, instance_data_size, image_index_offset, pipeline, images);
	if (instance.batch_index >= instances.size())
		instances.emplace_back();
	auto& instance_vec = instances[instance.batch_index];
	instance_vec.emplace_back(std::move(instance));
}

void DebugRenderer::init()
{
	render_context.init();
	immediate_render_context.init();
	global_uniform_buffer = makeShared<Buffer>(sizeof(GlobalUniformData), BufferUsage::UNIFORM | BufferUsage::TRANSFER_DST, Allocation::Props{ Allocation::MAPPED | Allocation::SEQUENTIAL_WRITE });

	VkRenderPass render_pass = RenderContext::getRenderGraph().getPass("Geometry").getRenderPass();

	graphics_pipeline_3D = makeShared<GraphicsPipeline>();
	graphics_pipeline_3D->setShader(makeShared<Shader>("3D", Shader::Defines{
		{ "MAX_IMAGE_SLOTS", std::to_string(DebugRenderer::MAX_IMAGE_SLOTS) },
		{ "MAX_LIGHTS", std::to_string(DebugRenderer::MAX_LIGHTS) } }))
		.setRenderPass(render_pass)
		.setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_WRITE)
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_TEST)
		.enableTag(GraphicsPipeline::EnableTag::BLEND)
		.setDepthCompareOp(GraphicsPipeline::CompareOp::LESS)
		.build();
	GraphicsPipeline::add("3D", graphics_pipeline_3D);

	graphics_pipeline_2D = makeShared<GraphicsPipeline>();
	graphics_pipeline_2D->setShader(makeShared<Shader>("2D", Shader::Defines {
		{ "MAX_IMAGE_SLOTS", std::to_string(DebugRenderer::MAX_IMAGE_SLOTS) } }))
		.setRenderPass(render_pass)
		.setSamples(RenderContext::getPhysicalDevice().getMaxSampleCount())
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_WRITE)
		.enableTag(GraphicsPipeline::EnableTag::DEPTH_TEST)
		.enableTag(GraphicsPipeline::EnableTag::BLEND)
		.setDepthCompareOp(GraphicsPipeline::CompareOp::GREATER_OR_EQUAL)
		.build();
	GraphicsPipeline::add("2D", graphics_pipeline_2D);

	Image::Props image_props{};
	image_props.width = 1;
	image_props.height = 1;
	image_props.sampler_props.min_filter = Filter::NEAREST;
	image_props.sampler_props.mag_filter = Filter::NEAREST;
	image_props.sampler_props.anisotropy = 1.0f;
	constexpr u8vec4 white(255);
	white_image = makeShared<Image>(image_props);
	white_image->setData(&white);
	white_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	Image::add("White", white_image);

	constexpr u8vec4 black(0);
	auto black_image = makeShared<Image>(image_props);
	black_image->setData(&black);
	black_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	Image::add("Black", black_image);

	image_props.width = 2;
	image_props.height = 2;
	image_props.sampler_props.u_wrap = Wrap::CLAMP_TO_EDGE;
	image_props.sampler_props.v_wrap = Wrap::CLAMP_TO_EDGE;
	constexpr u8vec4 null_data[4] = { { 0, 0, 0, 255 }, { 255, 0, 255, 255 }, { 255, 0, 255, 255 }, { 0, 0, 0, 255 } };
	auto null_image = makeShared<Image>(image_props);
	null_image->setData(null_data);
	null_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	Image::add("Null", null_image);

	Mesh::add("Triangle", makeShared<Mesh>(TriangleMesh()));
	Mesh::add("Circle", makeShared<Mesh>(CircleMesh()));
	Mesh::add("Circle Outline", makeShared<Mesh>(CircleOutlineMesh()));
	Mesh::add("Rectangle", makeShared<Mesh>(RectangleMesh()));
	Mesh::add("Rounded Rectangle", makeShared<Mesh>(RoundedRectangleMesh()));
	Mesh::add("Quad", makeShared<Mesh>(QuadMesh()));
	Mesh::add("Cube", makeShared<Mesh>(CubeMesh()));
	Mesh::add("Sphere", makeShared<Mesh>(SphereMesh()));

	Font::add("Arial", makeShared<Font>("res/fonts/arial.ttf"));

	reset();
}

void DebugRenderer::destroy()
{
	render_context.destroy();
	immediate_render_context.destroy();
	active = {};
	global_uniform_buffer = nullptr;
	graphics_pipeline_2D = nullptr;
	graphics_pipeline_3D = nullptr;
	white_image = nullptr;
}

void DebugRenderer::reset()
{
	immediate_render_context.clear();
	active = {};
	active.images = { white_image };
	active.font = Font::get("Arial");
}

void DebugRenderer::update(Camera* camera)
{
	stats = {};

	// Update uniforms
	if (camera)
	{
		global_uniform_data.projection_view = camera->projection_view;
		global_uniform_data.projection = camera->projection;
		global_uniform_data.view = camera->view;
		global_uniform_data.camera_position = camera->position;
		global_uniform_data.camera_direction = camera->direction;
	}
	global_uniform_data.projection_view2D = math::ortho(0.0f, (float)Window::get().getWidth(), 0.0f, (float)Window::get().getHeight(), 0.0f, 1.0f);
	global_uniform_data.delta_time = Time::dt;
	global_uniform_data.time = Time::runtime;
	global_uniform_data.frame = Time::frame;
	global_uniform_data.resolution = uvec2(Window::get().getWidth(), Window::get().getHeight());
	global_uniform_data.lights = lights;
	global_uniform_buffer->setData(&global_uniform_data, sizeof(GlobalUniformData));

	render_context.update();
	immediate_render_context.update();
}

void DebugRenderer::render()
{
	render_context.render();
	immediate_render_context.render();
}

#pragma region Primitives

void DebugRenderer::triangle(float x, float y, float width, float height)
{
	draw2D(Mesh::get("Triangle"), x, y, width, height);
}

void DebugRenderer::triangle(float x, float y, float size)
{
	triangle(x, y, size, size);
}

void DebugRenderer::triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	draw2D(makeShared<Mesh>(TriangleMesh(x1, y1, x2, y2, x3, y3)), 0, 0, 1, 1);
}

void DebugRenderer::rectangle(float x, float y, float width, float height)
{
	draw2D(Mesh::get("Rectangle"), x, y, width, height);
}

void DebugRenderer::roundedRectangle(float x, float y, float width, float height)
{
	draw2D(Mesh::get("Rounded Rectangle"), x, y, width, height);
}

void DebugRenderer::square(float x, float y, float size)
{
	rectangle(x, y, size, size);
}

void DebugRenderer::roundedSquare(float x, float y, float size)
{
	draw2D(Mesh::get("Rounded Rectangle"), x, y, size, size);
}

void DebugRenderer::ellipse(float x, float y, float width, float height)
{
	draw2D(Mesh::get("Circle"), x, y, width, height);
}

void DebugRenderer::ellipseOutline(float x, float y, float width, float height)
{
	draw2D(Mesh::get("Circle Outline"), x, y, width, height);
}

void DebugRenderer::circle(float x, float y, float radius)
{
	ellipse(x, y, radius, radius);
}

void DebugRenderer::circleOutline(float x, float y, float radius)
{
	ellipseOutline(x, y, radius, radius);
}

void DebugRenderer::line(const std::vector<vec2>& points, float width)
{
	draw2D(makeShared<Mesh>(LineMesh(points, width)), 0, 0, 1, 1);
}

void DebugRenderer::line(float x1, float y1, float x2, float y2, float width)
{
	float l = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	float dx = (x2 - x1) / l;
	float dy = (y2 - y1) / l;
	draw2D(Mesh::get("Rectangle"), {
			dx * l, dy * l, 0, 0,
			-dy * width, dx * width, 0, 0,
			0, 0, 1, 0,
			x1 + dy * width * 0.5f, y1 - dx * width * 0.5f, 1, 1
		});
}

void DebugRenderer::bezier(float x1, float y1, float px, float py, float x2, float y2, float width)
{
	draw2D(makeShared<Mesh>(BezierMesh(x1, y1, px, py, x2, y2, 64u, width)), 0, 0, 1, 1);
}

void DebugRenderer::bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width)
{
	draw2D(makeShared<Mesh>(BezierMesh(x1, y1, px1, py1, px2, py2, x2, y2, 64u, width)), 0, 0, 1, 1);
}

void DebugRenderer::text(const std::string& text, float x, float y, float width, float height)
{
	active.images = { active.font->getAtlas() };
	draw2D(makeShared<Mesh>(TextMesh(text, active.font)), x, y, width, height);
}

void DebugRenderer::text(const std::string& text, float x, float y, float size)
{
	active.images = { active.font->getAtlas() };
	draw2D(makeShared<Mesh>(TextMesh(text, active.font)), x, y, size, size);
}

void DebugRenderer::image(const shared<Image>& image, float x, float y, float width, float height)
{
	active.images = { image };
	draw2D(Mesh::get("Rectangle"), x, y, width, height);
}

void DebugRenderer::image(const shared<Image>& image, float x, float y, float size)
{
	active.images = { image };
	draw2D(Mesh::get("Rectangle"), x, y, size, size);
}

void DebugRenderer::mesh(const shared<Mesh>& mesh, float x, float y, float width, float height)
{
	draw2D(mesh, x, y, width, height);
}

void DebugRenderer::tetrahedron(float x, float y, float z, float size)
{
	draw3D(Mesh::get("Tetrahedron"), x, y, z, size, size, size);
}

void DebugRenderer::cube(float x, float y, float z, float size)
{
	draw3D(Mesh::get("Cube"), x, y, z, size, size, size);
}

void DebugRenderer::cuboid(float x, float y, float z, float width, float height, float depth)
{
	draw3D(Mesh::get("Cube"), x, y, z, width, height, depth);
}

void DebugRenderer::sphere(float x, float y, float z, float radius)
{
	draw3D(Mesh::get("Sphere"), x, y, z, radius, radius, radius);
}

void DebugRenderer::ellipsoid(float x, float y, float z, float width, float height, float depth)
{
	draw3D(Mesh::get("Sphere"), x, y, z, width, height, depth);
}

void DebugRenderer::mesh(const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth)
{
	draw3D(mesh, x, y, z, width, height, depth);
}

void DebugRenderer::model(const shared<Model>& model, float x, float y, float z, float width, float height, float depth)
{
	InstanceData3D data{};
	mat4 transform = {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, depth, 0,
		x, y, z, 1
	};
	data.color = active.color;
	if (active.transformed)
		transform *= active.transform;
	for (const auto& node : model->getNodes())
	{
		data.transform = transform * node.transform;
		for (const auto& primitive : node.primitives)
		{
			const auto& material = model->getMaterials()[primitive.material_index];
			data.metallic = material.metallic;
			data.roughness = material.roughness;
			data.emissive = material.emissive;
			immediate_render_context.createInstance(model->getMesh(), primitive.first_index, primitive.index_count, &data, sizeof(data), offsetof(data, image_index), graphics_pipeline_3D, model->getImages());
		}
	}
}
#pragma endregion

void DebugRenderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, const void* instance_data, size_t instance_data_size, uint32_t image_index_offset, const std::vector<shared<Image>>& images)
{
	immediate_render_context.createInstance(mesh, 0, mesh->getIndexCount(), instance_data, instance_data_size, image_index_offset, graphics_pipeline, images);
}

void DebugRenderer::draw3D(const shared<Mesh>& mesh, const mat4& transform)
{
	InstanceData3D data{};
	data.transform = transform;
	data.color = active.color;
	if (active.transformed)
		data.transform *= active.transform;
	draw(graphics_pipeline_3D, mesh, &data, sizeof(data), offsetof(data, image_index), active.images);
}

void DebugRenderer::draw3D(const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth)
{
	draw3D(mesh, {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, depth, 0,
		x, y, z, 1
	});
}

void DebugRenderer::draw2D(const shared<Mesh>& mesh, const mat4& transform)
{
	InstanceData2D data{};
	data.transform = transform;
	data.transform[3][3] = active.depth;
	data.color = active.color;
	if (active.transformed)
		data.transform *= active.transform;
	draw(graphics_pipeline_2D, mesh, &data, sizeof(data), offsetof(data, image_index), active.images);
	active.depth += 1e-8;
}

void DebugRenderer::draw2D(const shared<Mesh>& mesh, float x, float y, float width, float height)
{
	draw2D(mesh, {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, 1, 0,
		x, y, active.depth, 1
	});
}

Light* DebugRenderer::addLight(const Light& light)
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