#pragma once

#include "silk_engine/utils/color.h"
#include "silk_engine/scene/light.h"
#include "silk_engine/scene/instance_images.h"
#include "silk_engine/scene/camera/camera.h"

class Image;
class Font;
class Buffer;
class Mesh;
class Model;
class GraphicsPipeline;
class Material;

class DebugRenderer
{
public:
	static constexpr uint32_t MAX_IMAGE_SLOTS = 32;
	static constexpr uint32_t MAX_LIGHTS = 32;

	struct InstanceData2D
	{
		mat4 transform = mat4(1);
		vec4 color = vec4(1);
		int32_t image_index = -1;
	};

	struct InstanceData3D
	{
		mat4 transform = mat4(1);
		vec4 color = vec4(1);
		int32_t image_index = -1;

		float metallic = 1.0f;
		float roughness = 1.0f;
		vec3 emissive = vec3(0);
	};

	struct Renderable
	{
		size_t data_offset = std::numeric_limits<size_t>::max();
		size_t batch_index = std::numeric_limits<size_t>::max();
		size_t image_index = 0;
		size_t image_count = 0;
	};

private:
	static inline struct Active
	{
		vec4 color = vec4(1);
		std::vector<shared<Image>> images{};
		shared<Font> font = nullptr;
		mat4 transform = mat4(1);
		bool transformed = false;
		float depth = 1.0f;
	} active;

	// TODO: move this in command buffer? and keep statistics only on debug mode
	static inline struct Statistics
	{
		size_t instance_batches = 0;
		size_t instances = 0;
		size_t vertices = 0;
		size_t indices = 0;
	} stats{};

	struct GlobalUniformData
	{
		mat4 projection_view;
		mat4 projection_view2D;
		mat4 projection;
		mat4 view;
		vec3 camera_position;
		float time;
		vec3 camera_direction;
		float delta_time;
		uvec2 resolution;
		uint32_t frame;
		uint32_t padding;
		std::array<Light, MAX_LIGHTS> lights;
	};

	struct CullData
	{
		vec3 min;
		uint32_t index;
		vec3 max;
		uint32_t count;
		std::array<vec4, 6> planes;
	};

	class InstancedRendererBase
	{
		struct InstanceGroup
		{
		public:
			void bind();
			void setData(size_t offset, const void* data)
			{ 
				memcpy(instance_data.data() + offset, data, data_size);
				needs_update = true; 
			}
			void addData(const void* data) 
			{
				instance_data.resize(instance_data.size() + data_size);
				setData(instance_data.size() - data_size, data);
				++instance_count;
			}
			void clear()
			{
				instance_data.clear();
				instance_images.clear();
				instance_count = 0;
			}

		public:
			shared<Mesh> mesh = nullptr;
			uint32_t first_index = 0;
			uint32_t index_count = 0;
			std::vector<uint8_t> instance_data{};
			InstanceImages instance_images{};
			shared<Material> material = nullptr;
			shared<Buffer> instance_buffer = nullptr;
			shared<Buffer> old_instance_buffer = nullptr;
			bool needs_update = false;
			size_t instance_count = 0;
			size_t data_size = 0;
			size_t hash = 0;
		};
	public:
		void init();
		void destroy()
		{
			instance_batches.clear();
			draw_commands.clear();
			indirect_buffer = nullptr;
		}

		void update();
		void render();

		// @param first_index for indexed mesh otherwise first_index is first_vertex
		// @param index_count for indexed mesh otherwise index_count is vertex_count
		Renderable createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const void* instance_data, size_t instance_data_size, size_t image_index_offset = std::numeric_limits<size_t>::max(), const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {});
		void updateInstance(const Renderable& instance, const void* instance_data)
		{
			instance_batches[instance.batch_index].setData(instance.data_offset, instance_data);
		}

	public:
		shared<Buffer> indirect_buffer;
		std::vector<VkDrawIndexedIndirectCommand> draw_commands;
		std::vector<InstanceGroup> instance_batches;
	};

	class InstancedRenderer : public InstancedRendererBase
	{
	public:
		shared<Renderable> createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const void* instance_data, size_t instance_data_size, size_t image_index_offset = std::numeric_limits<size_t>::max(), const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {});
		void destroyInstance(const Renderable& instance);

	private:
		std::vector<std::vector<shared<Renderable>>> instances;
	};

	class ImmediateInstancedRenderer : public InstancedRendererBase
	{
	public:
		void clear()
		{
			bool any_needs_update = false;
			for (int i = 0; i < instance_batches.size(); ++i)
			{
				bool should_remove = instance_batches[i].instance_count < 16;
				instance_batches[i].clear();
				if (should_remove)
				{
					std::swap(instance_batches[i], instance_batches.back());
					std::swap(draw_commands[i], draw_commands.back());
					std::swap(instances[i], instances.back());
					instance_batches.pop_back();
					draw_commands.pop_back();
					instances.pop_back(); 
					--i;
					any_needs_update = true;
				}
			}
			for (auto& instance : instances)
				instance.clear();
		}

		void createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const void* instance_data, size_t instance_data_size, size_t image_index_offset = std::numeric_limits<size_t>::max(), const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {});

	private:
		std::vector<std::vector<Renderable>> instances;
	};

public:
	static void init();
	static void destroy();
	static void reset();
	static void update(Camera* camera);
	static void render();

	static const Active& getActive() { return active; }
	static const shared<Image>& getWhiteImage() { return white_image; }
	static const shared<Buffer>& getGlobalUniformBuffer() { return global_uniform_buffer; }

	// States
	static void transform(const mat4& transform = mat4(1)) { active.transformed = transform != mat4(1); active.transform = transform; }
	static void color(const Color& color = Colors::WHITE) { active.color = color; }
	static void image(const shared<Image>& image = white_image) { active.images[0] = image; }
	static void depth(float depth) { active.depth = depth; }

	// 2D
	static void triangle(float x, float y, float width, float height);
	static void triangle(float x, float y, float size);
	static void triangle(float x1, float y1, float x2, float y2, float x3, float y3);
	static void rectangle(float x, float y, float width, float height);
	static void roundedRectangle(float x, float y, float width, float height);
	static void square(float x, float y, float size);
	static void roundedSquare(float x, float y, float size);
	static void ellipse(float x, float y, float width, float height);
	static void ellipseOutline(float x, float y, float width, float height);
	static void circle(float x, float y, float radius);
	static void circleOutline(float x, float y, float radius);
	static void line(const std::vector<vec2>& points, float width = 1.0f);
	static void line(float x1, float y1, float x2, float y2, float width = 1.0f);
	static void bezier(float x1, float y1, float px, float py, float x2, float y2, float width = 1.0f);
	static void bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width = 1.0f);
	static void text(const std::string& text, float x, float y, float width, float height);
	static void text(const std::string& text, float x, float y, float size);
	static void image(const shared<Image>& image, float x, float y, float width, float height);
	static void image(const shared<Image>& image, float x, float y, float size);
	static void mesh(const shared<Mesh>& mesh, float x, float y, float width, float height);

	// 3D
	static void tetrahedron(float x, float y, float z, float size);
	static void cube(float x, float y, float z, float size);
	static void cuboid(float x, float y, float z, float width, float height, float depth);
	static void sphere(float x, float y, float z, float radius);
	static void ellipsoid(float x, float y, float z, float width, float height, float depth);
	static void mesh(const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth);
	static void model(const shared<Model>& model, float x, float y, float z, float width, float height, float depth);

	// Slow function for quickly drawing stuff
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, const void* instance_data, size_t instance_data_size, uint32_t image_index_offset = std::numeric_limits<size_t>::max(), const std::vector<shared<Image>>& images = {});
	static void draw3D(const shared<Mesh>& mesh, const mat4& transform);
	static void draw3D(const shared<Mesh>& mesh, float x, float y, float z, float width = 1.0f, float height = 1.0f, float depth = 1.0f);
	static void draw2D(const shared<Mesh>& mesh, const mat4& transform);
	static void draw2D(const shared<Mesh>& mesh, float x, float y, float width, float height);

	static Light* addLight(const Light& light);

	static shared<Renderable> createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const void* instance_data, size_t instance_data_size, uint32_t image_index_offset = std::numeric_limits<size_t>::max(), const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {}) { return render_context.createInstance(mesh, first_index, index_count, instance_data, instance_data_size, image_index_offset, pipeline, images); }
	static shared<Renderable> createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const InstanceData2D& instance_data, const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {}) { return createInstance(mesh, first_index, index_count, &instance_data, sizeof(instance_data), offsetof(instance_data, image_index), pipeline, images); }
	static shared<Renderable> createInstance(const shared<Mesh>& mesh, uint32_t first_index, uint32_t index_count, const InstanceData3D& instance_data, const shared<GraphicsPipeline>& pipeline = nullptr, const std::vector<shared<Image>>& images = {}) { return createInstance(mesh, first_index, index_count, &instance_data, sizeof(instance_data), offsetof(instance_data, image_index), pipeline, images); }
	static void updateInstance(const Renderable& instance, const void* instance_data) { render_context.updateInstance(instance, instance_data); }
	static void destroyInstance(const Renderable& instance) { render_context.destroyInstance(instance); }

private:
	static inline InstancedRenderer render_context;
	static inline ImmediateInstancedRenderer immediate_render_context;
	static shared<Buffer> global_uniform_buffer;
	static inline GlobalUniformData global_uniform_data{};
	static std::array<Light, MAX_LIGHTS> lights;
	static shared<Image> white_image;
	static shared<GraphicsPipeline> graphics_pipeline_2D;
	static shared<GraphicsPipeline> graphics_pipeline_3D;
};