#pragma once

#include "utils/color.h"
#include "scene/instance_images.h"
#include "scene/resources.h"
#include "scene/light.h"

class Image;
class Font;
class GraphicsPipeline;
class Buffer;
class VertexBuffer;
class InstanceImages;
class Mesh;
class DescriptorSet;
class Camera;
class RenderedInstance;
class InstanceBatch;

class DebugRenderer
{
	// TODO: Fix this
	friend class MeshSubrender;
	friend class ParticleSystem;
public:
	static constexpr size_t MAX_INSTANCE_BATCHES = 8192;
	static constexpr size_t MAX_INSTANCES = 8192;
	static constexpr size_t MAX_IMAGE_SLOTS = 32; // Can be more, but will be slower
	static constexpr size_t MAX_LIGHTS = 64;

private:
	static inline struct Active
	{
		vec4 color = vec4(1);
		shared<Image> image = nullptr;
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

	struct InstanceData
	{
		mat4 transform = mat4(1);
		uint32_t image_index = 0;
		vec4 color = vec4(1);

		bool operator==(const InstanceData& other) const;
	};

	struct RenderedInstance
	{
		shared<GraphicsPipeline> pipeline = nullptr;
		std::vector<shared<Image>> images;
		size_t instance_data_index = std::numeric_limits<size_t>::max();
		size_t instance_batch_index = std::numeric_limits<size_t>::max();

		bool operator==(const RenderedInstance& other) const;
	};

	struct InstanceBatch
	{
		shared<Mesh> mesh = nullptr;
		shared<RenderedInstance> instance = nullptr;

		std::vector<InstanceData> instance_data;
		std::vector<shared<RenderedInstance>> instances;
		shared<VertexBuffer> instance_buffer = nullptr;
		InstanceImages instance_images{};
		std::unordered_map<uint32_t, shared<DescriptorSet>> descriptor_sets;

		~InstanceBatch();

		void bind();

		bool needs_update = true;

		bool operator==(const RenderedInstance& instance) const;
	};

public:
	static void init();
	static void destroy();
	static void reset();
	static void update(Camera* camera);
	
	static const Active& getActive() { return active; }

	//States
	static void transform(const mat4& transform = mat4(1)) { active.transformed = transform != mat4(1); active.transform = transform; }
	static void color(const Color& color = Colors::WHITE) { active.color = color; }
	static void image(const shared<Image>& image = Resources::white_image) { active.image = image; }
	static void depth(float depth) { active.depth = depth; }

	//2D
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
	static void bezier(float x1, float y1, float px, float py, float x2, float y2, float width);
	static void bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width);
	static void text(const std::string& text, float x, float y, float width, float height);
	static void text(const std::string& text, float x, float y, float size);
	static void image(const shared<Image>& image, float x, float y, float width, float height);
	static void image(const shared<Image>& image, float x, float y, float size);

	//3D
	static void tetrahedron(float x, float y, float z, float size);
	static void cube(float x, float y, float z, float size);
	static void cuboid(float x, float y, float z, float width, float height, float depth);
	static void sphere(float x, float y, float z, float radius);
	static void ellipsoid(float x, float y, float z, float width, float height, float depth);

	//Slow function for quickly drawing stuff
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, const mat4& transform, const std::vector<shared<Image>>& images = {});
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth = 1.0f, const std::vector<shared<Image>>& images = {});
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float width, float height, const std::vector<shared<Image>>& images = {});

	static Light* addLight(const Light& light);
	static void createInstance(const shared<RenderedInstance>& instance, const shared<Mesh>& mesh, const InstanceData& instance_data);
	static void updateInstance(RenderedInstance& instance, const InstanceData& instance_data);
	static void addInstanceBatch(const shared<RenderedInstance>& instance, const shared<Mesh>& mesh, const InstanceData& instance_data);
	static void destroyInstance(const RenderedInstance& instance);

private:
	static void updateUniformData(Camera* camera);
	static void updateDrawCommands();

private:
	static std::vector<InstanceBatch> instance_batches;
	static std::vector<shared<RenderedInstance>> instances;
	static unique<Buffer> indirect_buffer;
	static unique<Buffer> global_uniform_buffer;
	static std::array<Light, MAX_LIGHTS> lights;
};