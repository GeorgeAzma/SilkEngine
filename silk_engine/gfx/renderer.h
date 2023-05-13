#pragma once

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

private:
	static inline std::vector<shared<RenderPass>> render_passes;
	static inline Fence* previous_frame_finished = nullptr;
	static inline Semaphore* swap_chain_image_available = nullptr;
	static inline Semaphore* render_finished = nullptr;
};