#include "debug_renderer.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"
#include "scene/meshes/mesh.h"
#include "scene/meshes/bezier_mesh.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/text_mesh.h"
#include "scene/meshes/triangle_mesh.h"
#include "scene/meshes/circle_mesh.h"
#include "scene/meshes/circle_outline_mesh.h"
#include "scene/meshes/rectangle_mesh.h"
#include "scene/meshes/rounded_rectangle_mesh.h"
#include "scene/meshes/quad_mesh.h"
#include "scene/meshes/cube_mesh.h"
#include "scene/meshes/sphere_mesh.h"
#include "window/window.h"
#include "material.h"

std::vector<DebugRenderer::InstanceBatch> DebugRenderer::instance_batches{};
std::vector<shared<DebugRenderer::RenderedInstance>> DebugRenderer::instances{};
shared<Buffer> DebugRenderer::indirect_buffer = nullptr;
shared<Buffer> DebugRenderer::global_uniform_buffer = nullptr;
std::array<Light, DebugRenderer::MAX_LIGHTS> DebugRenderer::lights{};
shared<Image> DebugRenderer::white_image = nullptr;
shared<GraphicsPipeline> DebugRenderer::pipeline_2D = nullptr;
shared<GraphicsPipeline> DebugRenderer::pipeline_3D = nullptr;

bool DebugRenderer::InstanceData::operator==(const InstanceData& other) const
{
	return transform == other.transform
		&& image_index == other.image_index
		&& color == color;
}

void DebugRenderer::InstanceBatch::bind()
{
	material->bind();
	mesh->getVertexArray()->bind();
	instance_buffer->bind(1);
}

void DebugRenderer::init()
{
	instance_batches.reserve(MAX_INSTANCE_BATCHES); //TODO: remove this limitation some time
	indirect_buffer = makeShared<Buffer>(MAX_INSTANCE_BATCHES * sizeof(VkDrawIndexedIndirectCommand), Buffer::INDIRECT | Buffer::TRANSFER_DST, Allocation::Props{ Allocation::MAPPED | Allocation::RANDOM_ACCESS, Allocation::Device::CPU });
	global_uniform_buffer = makeShared<Buffer>(sizeof(GlobalUniformData), Buffer::UNIFORM | Buffer::TRANSFER_DST, Allocation::Props{ Allocation::MAPPED | Allocation::SEQUENTIAL_WRITE });
	pipeline_2D = GraphicsPipeline::get("2D");
	pipeline_3D = GraphicsPipeline::get("3D");

	Image::Props image_props{};
	image_props.width = 1;
	image_props.height = 1;
	image_props.sampler_props.min_filter = VK_FILTER_NEAREST;
	image_props.sampler_props.mag_filter = VK_FILTER_NEAREST;
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
	image_props.sampler_props.u_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	image_props.sampler_props.v_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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
	
	Font::add("Arial", makeShared<Font>("arial.ttf"));
	
	reset();
}

void DebugRenderer::destroy()
{
	active = {};
	instances.clear();
	instance_batches.clear();
	indirect_buffer = nullptr;
	global_uniform_buffer = nullptr;
}

void DebugRenderer::reset()
{
	for (const auto& instance : instances)
		destroyInstance(*instance);
	instances.clear();
	instances.reserve(MAX_INSTANCES);
	active = {};
	active.images = { white_image };
	active.font = Font::get("Arial");
}

void DebugRenderer::update(Camera* camera)
{
	stats = {};

	// Update uniforms
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

	// Update draw commands
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

void DebugRenderer::render()
{
	uint32_t draw_index = 0;
	for (auto& instance_batch : instance_batches)
	{
		instance_batch.material->set("GlobalUniform", *global_uniform_buffer);
		instance_batch.material->set("images", instance_batch.instance_images.getDescriptorImageInfos());
		instance_batch.bind();
		indirect_buffer->drawIndexedIndirect(draw_index);
		++draw_index;
	}
}

void DebugRenderer::triangle(float x, float y, float width, float height)
{
	draw(pipeline_2D, Mesh::get("Triangle"), x, y, width, height);
}

void DebugRenderer::triangle(float x, float y, float size)
{
	triangle(x, y, size, size);
}

void DebugRenderer::triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	draw(pipeline_2D, makeShared<Mesh>(TriangleMesh(x1, y1, x2, y2, x3, y3)), 0, 0, 1, 1);
}

void DebugRenderer::rectangle(float x, float y, float width, float height)
{
	draw(pipeline_2D, Mesh::get("Rectangle"), x, y, width, height);
}

void DebugRenderer::roundedRectangle(float x, float y, float width, float height)
{
	draw(pipeline_2D, Mesh::get("Rounded Rectangle"), x, y, width, height);
}

void DebugRenderer::square(float x, float y, float size)
{
	rectangle(x, y, size, size);
}

void DebugRenderer::roundedSquare(float x, float y, float size)
{
	draw(pipeline_2D, Mesh::get("Rounded Rectangle"), x, y, size, size);
}

void DebugRenderer::ellipse(float x, float y, float width, float height)
{
	draw(pipeline_2D, Mesh::get("Circle"), x, y, width, height);
}

void DebugRenderer::ellipseOutline(float x, float y, float width, float height)
{
	draw(pipeline_2D, Mesh::get("Circle Outline"), x, y, width, height);
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
	draw(pipeline_2D, makeShared<Mesh>(LineMesh(points, width)), 0, 0, 1, 1);
}

void DebugRenderer::line(float x1, float y1, float x2, float y2, float width)
{
	float l = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	float dx = (x2 - x1) / l;
	float dy = (y2 - y1) / l;
	draw(pipeline_2D, Mesh::get("Rectangle"), {
			dx * l, dy * l, 0, 0,
			-dy * width, dx * width, 0, 0,
			0, 0, 1, 0,
			x1 + dy * width * 0.5f, y1 - dx * width * 0.5f, 1, 1
		});
}

void DebugRenderer::bezier(float x1, float y1, float px, float py, float x2, float y2, float width)
{
	draw(pipeline_2D, makeShared<Mesh>(BezierMesh(x1, y1, px, py, x2, y2, 64u, width)), 0, 0, 1, 1);
}

void DebugRenderer::bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width)
{
	draw(pipeline_2D, makeShared<Mesh>(BezierMesh(x1, y1, px1, py1, px2, py2, x2, y2, 64u, width)), 0, 0, 1, 1);
}

void DebugRenderer::text(const std::string& text, float x, float y, float width, float height)
{
	active.images = { active.font->getAtlas() };
	draw(GraphicsPipeline::get("Font"), makeShared<Mesh>(TextMesh(text, 64, active.font)), x, y, width, height);
}

void DebugRenderer::text(const std::string& text, float x, float y, float size)
{
	active.images = { active.font->getAtlas() };
	draw(GraphicsPipeline::get("Font"), makeShared<Mesh>(TextMesh(text, 32, active.font)), x, y, size, size);
}

void DebugRenderer::tetrahedron(float x, float y, float z, float size)
{
	draw(pipeline_3D, Mesh::get("Tetrahedron"), x, y, z, size, size, size);
}

void DebugRenderer::cube(float x, float y, float z, float size)
{
	draw(pipeline_3D, Mesh::get("Cube"), x, y, z, size, size, size);
}

void DebugRenderer::cuboid(float x, float y, float z, float width, float height, float depth)
{
	draw(pipeline_3D, Mesh::get("Cube"), x, y, z, width, height, depth);
}

void DebugRenderer::sphere(float x, float y, float z, float radius)
{
	draw(pipeline_3D, Mesh::get("Sphere"), x, y, z, radius, radius, radius);
}

void DebugRenderer::ellipsoid(float x, float y, float z, float width, float height, float depth)
{
	draw(pipeline_3D, Mesh::get("Sphere"), x, y, z, width, height, depth);
}

void DebugRenderer::image(const shared<Image>& image, float x, float y, float width, float height)
{
	active.images = { image };
	draw(pipeline_2D, Mesh::get("Rectangle"), x, y, width, height);
}

void DebugRenderer::image(const shared<Image>& image, float x, float y, float size)
{
	active.images = { image };
	draw(pipeline_2D, Mesh::get("Rectangle"), x, y, size, size);
}

void DebugRenderer::mesh(const shared<Mesh>& mesh, float x, float y, float width, float height)
{
	draw(pipeline_2D, mesh, x, y, width, height);
}

void DebugRenderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, const mat4& transform)
{
	InstanceData data;
	data.transform = transform;
	data.color = active.color;
	if (active.transformed)
		data.transform *= active.transform;
	instances.emplace_back(std::move(createInstance(mesh, std::move(data), graphics_pipeline, active.images)));
}

void DebugRenderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth)
{
	InstanceData data;
	data.transform = {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, depth, 0,
		x, y, z, 1
	};
	data.color = active.color;
	if (active.transformed)
		data.transform *= active.transform;
	instances.emplace_back(std::move(createInstance(mesh, std::move(data), graphics_pipeline, active.images)));
}

void DebugRenderer::draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float width, float height)
{
	InstanceData data;
	data.transform = {
		width, 0, 0, 0,
		0, height, 0, 0,
		0, 0, 1, 0,
		x, y, active.depth, 1
	};
	data.color = active.color;
	if (active.transformed)
		data.transform *= active.transform;
	instances.emplace_back(std::move(createInstance(mesh, std::move(data), graphics_pipeline, active.images)));
	active.depth -= 1e-10;
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

shared<DebugRenderer::RenderedInstance> DebugRenderer::createInstance(const shared<Mesh>& mesh, const InstanceData& instance_data, const shared<GraphicsPipeline>& pipeline, const std::vector<shared<Image>>& images)
{
	shared<RenderedInstance> instance = nullptr;
	bool need_new_instance_batch = true;
	for (size_t i = 0; i < instance_batches.size(); ++i)
	{
		auto& instance_batch = instance_batches[i];
		if (instance_batch.material->getPipeline() == pipeline && instance_batch.mesh == mesh && instance_batch.instance_data.size() < MAX_INSTANCES && instance_batch.instance_images.available() >= images.size())
		{
			instance_batch.needs_update = true;
			instance = makeShared<RenderedInstance>(images.size(), instance_batch.instance_data.size(), i);
			instance_batch.instance_data.emplace_back(instance_data);
			instance_batch.instances.emplace_back(instance);
			break;
		}
	}

	if (!instance)
	{
		instance = makeShared<RenderedInstance>(images.size(), 0, instance_batches.size());
		addInstanceBatch(instance, pipeline, mesh, instance_data);
	}

	if (images.size())
	{
		uint32_t image_index = instance_batches[instance->instance_batch_index].instance_images.add(images);
		SK_VERIFY(image_index != UINT32_MAX, "Instance has too much images");
		instance_batches[instance->instance_batch_index].instance_data[instance->instance_data_index].image_index = image_index;
	}

	return instance;
}

void DebugRenderer::updateInstance(const RenderedInstance& instance, const InstanceData& instance_data)
{
	instance_batches[instance.instance_batch_index].needs_update = true;
	instance_batches[instance.instance_batch_index].instance_data[instance.instance_data_index] = instance_data;
}

void DebugRenderer::addInstanceBatch(const shared<RenderedInstance>& instance, const shared<GraphicsPipeline>& pipeline, const shared<Mesh>& mesh, const InstanceData& instance_data)
{
	instance_batches.emplace_back(mesh);
	auto& new_batch = instance_batches.back();
	new_batch.instance_images.add({ white_image });
	new_batch.instance_data.reserve(MAX_INSTANCES);
	new_batch.instance_data.emplace_back(instance_data);
	new_batch.instances.reserve(MAX_INSTANCES);
	new_batch.instances.emplace_back(instance);
	new_batch.instance_buffer = makeShared<VertexBuffer>(nullptr, sizeof(InstanceData), MAX_INSTANCES, true);
	new_batch.material = makeShared<Material>(pipeline);
}

void DebugRenderer::destroyInstance(const RenderedInstance& instance)
{
	auto& instance_batch = instance_batches[instance.instance_batch_index];

	instance_batch.instances.back()->instance_data_index = instance.instance_data_index;

	if (instance_batch.instance_data.size() == 1)
	{
		if (instance.instance_batch_index != instance_batches.size() - 1)
		{
			InstanceBatch* last_batch = &instance_batch;
			std::swap(instance_batch, instance_batches.back());
			for (auto& i : last_batch->instances)
				i->instance_batch_index = instance.instance_batch_index;
		}
		instance_batches.pop_back();
		return;
	}

	instance_batch.instance_images.remove(instance_batch.instance_data[instance.instance_data_index].image_index, instance.image_count);

	instance_batch.needs_update = true;

	std::swap(instance_batch.instance_data[instance.instance_data_index], instance_batch.instance_data.back());
	instance_batch.instance_data.pop_back();

	std::swap(instance_batch.instances[instance.instance_data_index], instance_batch.instances.back());
	instance_batch.instances.pop_back();
}