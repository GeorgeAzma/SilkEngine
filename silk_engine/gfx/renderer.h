#pragma once

class RenderPipeline;
class VertexArray;
class Camera;
class RenderPass;
class Fence;
class Light;
class Mesh;

class Renderer
{
public:

public:
	static void init();
	static void render(Camera* camera = nullptr);
	static void destroy();
	static void wait();

	template<typename T>
	static void setRenderPipeline() { render_pipeline = makeUnique<T>(); }
	static RenderPipeline& getRenderPipeline() { return *render_pipeline; }
	static shared<RenderPass>& getRenderPass(uint32_t index);

private:
	static unique<RenderPipeline> render_pipeline;
	static Fence* previous_frame_finished;
	static VkSemaphore swap_chain_image_available;
	static VkSemaphore render_finished;
};