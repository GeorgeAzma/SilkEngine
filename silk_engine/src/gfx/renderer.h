#pragma once

#include "utils/color.h"
#include "scene/resources.h"
#include "gfx/pipeline/render_pipeline.h"
#include "scene/light.h"
#include "scene/instance.h"
#include "buffers/uniform_buffer.h"

class CommandBuffer;
class IndirectBuffer;
class Camera;
class RenderPass;

class Renderer
{
	static inline struct Active
	{
		glm::vec4 color = glm::vec4(1);
		glm::vec4 stroke = glm::vec4(1);
		float stroke_weight = 1.0f;
		shared<Image2D> image = nullptr;
		shared<Font> font = nullptr;
		glm::mat4 transform = glm::mat4(1);
		bool transformed = false;
	} active;

public:
	static constexpr size_t MAX_LIGHTS = 64;

public:
	struct GlobalUniformData
	{
		glm::mat4 projection_view;
		glm::mat4 projection_view2D;
		glm::mat4 projection;
		glm::mat4 view;
		glm::vec3 camera_position;
		float time;
		glm::vec3 camera_direction;
		float delta_time;
		glm::uvec2 resolution;
		uint32_t frame;
		uint32_t padding;
		std::array<Light, MAX_LIGHTS> lights;
	};

public:
	static void init();
	static void waitForPreviousFrame();
	static void begin(Camera* camera = nullptr);
	static void render();
	static void end();
	static void destroy();
	static void reset();

	template<typename T>
	static void setRenderPipeline() { render_pipeline = makeUnique<T>(); }
	static RenderPipeline& getRenderPipeline() { return *render_pipeline; }
	static shared<RenderPass>& getRenderPass(uint32_t index) { return render_pipeline->getRenderStages()[index].getRenderPass(); }

	//States
	static void transform(const glm::mat4& transform = glm::mat4(1)) { active.transformed = transform != glm::mat4(1); active.transform = transform; }
	static void color(const Color& color = Colors::WHITE) { active.color = color; }
	static void stroke(const Color& stroke = Colors::WHITE) { active.stroke = stroke; }
	static void strokeWeight(float stroke_weight = 1.0f) { active.stroke_weight = stroke_weight; }
	static void image(const shared<Image2D>& image = Resources::white_image) { active.image = image; }

	//2D
	static void triangle(float x, float y, float width, float height);
	static void triangle(float x, float y, float size);
	static void triangle(float x1, float y1, float x2, float y2, float x3, float y3);
	static void rectangle(float x, float y, float width, float height);
	static void square(float x, float y, float size);
	static void ellipse(float x, float y, float width, float height);
	static void circle(float x, float y, float radius);
	static void line(const std::vector<glm::vec2>& points, float width = 1.0f);
	static void line(float x1, float y1, float x2, float y2, float width = 1.0f);
	static void bezier(float x1, float y1, float px, float py, float x2, float y2, float width);
	static void bezier(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, float width);
	static void text(const std::string& text, float x, float y, float width, float height);
	static void text(const std::string& text, float x, float y, float size);

	//3D
	static void tetrahedron(float x, float y, float z, float size);
	static void cube(float x, float y, float z, float size);
	static void cuboid(float x, float y, float z, float width, float height, float depth);
	static void sphere(float x, float y, float z, float radius);
	static void ellipsoid(float x, float y, float z, float width, float height, float depth);

	//Slow function for quickly drawing stuff
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, glm::mat4&& transform, std::initializer_list<shared<Image2D>>&& images, bool stroke = false);
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, glm::mat4&& transform, bool stroke = false);
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth, std::initializer_list<shared<Image2D>>&& images, bool stroke = false);
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth = 1.0f, bool stroke = false);
	
	static Light* addLight(const Light& light);
	static void createInstance(const shared<RenderedInstance>& instance, const shared<Mesh>& mesh, InstanceData&& instance_data);
	static void updateInstance(RenderedInstance& instance, const InstanceData& instance_data);
	static InstanceBatch& getInstanceBatch(size_t index) { return instance_batches[index]; }
	static std::vector<InstanceBatch>& getInstanceBatches() { return instance_batches; }
	static const unique<IndirectBuffer>& getIndirectBuffer() { return indirect_buffer;  }
	static const unique<UniformBuffer>& getGlobalUniformBuffer() { return global_uniform_buffer; }
	static void addInstanceBatch(const shared<RenderedInstance>& instance, const shared<Mesh>& mesh, InstanceData&& instance_data);
	static void destroyInstance(const RenderedInstance& instance);

private:
	static void updateUniformData(Camera* camera);
	static void updateDrawCommands();

private:
	static inline unique<RenderPipeline> render_pipeline = nullptr;
	static inline std::vector<InstanceBatch> instance_batches;
	static inline std::vector<shared<RenderedInstance>> instances;
	static inline unique<IndirectBuffer> indirect_buffer;
	static inline unique<UniformBuffer> global_uniform_buffer;
	static inline std::array<Light, MAX_LIGHTS> lights;

	static inline CommandBuffer* command_buffer = nullptr;
	static inline VkFence previous_frame_finished = VK_NULL_HANDLE;
	static inline VkSemaphore swap_chain_image_available = VK_NULL_HANDLE;
	static inline VkSemaphore render_finished = VK_NULL_HANDLE;
};