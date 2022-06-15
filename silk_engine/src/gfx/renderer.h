#pragma once

#include "scene/instance.h"
#include "scene/camera/camera.h"
#include "buffers/uniform_buffer.h"
#include "buffers/command_buffer.h"
#include "buffers/indirect_buffer.h"
#include "scene/light.h"
#include "utils/color.h"
#include "subrender.h"
#include "utils/type_info.h"
#include "scene/resources.h"

class Renderer
{
	static inline struct Active
	{
		glm::vec4 color;
		shared<Image2D> image;
		glm::mat4 transform;
		bool transformed;
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
	static void begin(Camera* camera = nullptr);
	static void render();
	static void end();
	static void cleanup();
	static void reset();

	template<typename T>
	static const T* getSubrender()
	{
		TypeID type_id = TypeInfo<Subrender>::getTypeID<T>();
		if (auto it = subrenders.find(type_id);  it != subrenders.end() && it->second)
			return (T*)(it->second.get());
		return nullptr;
	}

	template<typename T>
	static void addSubrender()
	{
		TypeID type_id = TypeInfo<Subrender>::getTypeID<T>();
		subrenders.emplace(type_id, makeShared<T>());
	}

	template<typename T>
	static void removeSubrender()
	{
		TypeID type_id = TypeInfo<Subrender>::getTypeID<T>();
		subrenders.erase(type_id);
	}

	static void clearSubrenders()
	{
		subrenders.clear();
	}

	//States
	static void transform(const glm::mat4& transform = glm::mat4(1)) { active.transformed = transform != glm::mat4(1); active.transform = transform; }
	static void color(const Color& color = Colors::WHITE) { active.color = color; }
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

	//3D
	static void tetrahedron(float x, float y, float z, float size);
	static void cube(float x, float y, float z, float size);
	static void cuboid(float x, float y, float z, float width, float height, float depth);
	static void sphere(float x, float y, float z, float radius);
	static void ellipsoid(float x, float y, float z, float width, float height, float depth);

	//Slow function for quickly drawing stuff
	static void draw(const shared<GraphicsPipeline>& graphics_pipeline, const shared<Mesh>& mesh, float x, float y, float z, float width, float height, float depth = 1.0f);

	static Light* addLight(const Light& light);
	static void createInstance(const shared<RenderedInstance>& instance, const InstanceData& instance_data);
	static void updateInstance(RenderedInstance& instance, const InstanceData& instance_data);
	static InstanceBatch& getInstanceBatch(size_t index) { return instance_batches[index]; }
	static void addInstanceBatch(const shared<RenderedInstance>& instance, const InstanceData& instance_data);
	static void destroyInstance(const RenderedInstance& instance);

private:
	static void updateUniformData(Camera* camera);
	static void updateDrawCommands();

private:
	static inline std::vector<InstanceBatch> instance_batches;
	static inline std::vector<shared<RenderedInstance>> instances;
	static inline unique<IndirectBuffer> indirect_buffer;
	static inline unique<UniformBuffer> global_uniform_buffer;
	static inline std::array<Light, MAX_LIGHTS> lights;

	static inline CommandBuffer* command_buffer = nullptr;
	static inline VkFence previous_frame_finished = VK_NULL_HANDLE;
	static inline VkSemaphore swap_chain_image_available = VK_NULL_HANDLE;
	static inline VkSemaphore render_finished = VK_NULL_HANDLE;
	static inline std::unordered_map<TypeID, shared<Subrender>> subrenders;
};