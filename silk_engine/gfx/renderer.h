#pragma once

class RenderPipeline;
class RenderPass;
class Fence;
class Semaphore;
class Mesh;
class Material;

class Renderer
{
public:
	static void init();
	static void render();
	static void destroy();
	static void wait();

	template<typename T>
	static void setRenderPipeline() 
	{ 
		if (render_pipeline)
		{
			delete render_pipeline; 
			render_pipeline = nullptr;
		} 
		render_pipeline = new T(); 
	}
	static RenderPipeline& getRenderPipeline() { return *render_pipeline; }
	static const RenderPass& getRenderPass(uint32_t index);

private:
	static inline RenderPipeline* render_pipeline = nullptr;
	static inline Fence* previous_frame_finished = nullptr;
	static inline Semaphore* swap_chain_image_available = nullptr;
	static inline Semaphore* render_finished = nullptr;
};