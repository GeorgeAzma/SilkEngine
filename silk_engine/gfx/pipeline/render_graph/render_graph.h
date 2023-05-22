#pragma once

#include "resource.h"
#include "pass.h"

class RenderPass;
class SwapChain;
class Fence;
class Semaphore;

class RenderGraph
{
public:
	Pass& addPass() 
	{ 
		passes.emplace_back(makeUnique<Pass>(Pass::Type::RENDER, *this)); 
		return *passes.back(); 
	}
	Pass& addComputePass()
	{
		passes.emplace_back(makeUnique<Pass>(Pass::Type::COMPUTE, *this));
		return *passes.back();
	}
	Resource& addResource(unique<Resource>&& resource) 
	{ 
		resources_map.emplace(resource->getName(), resource.get());
		resources.emplace_back(std::move(resource));
		return *resources.back();
	}

	void build(const char* backbuffer = nullptr);
	void render();
	void resize(const SwapChain& swap_chain);

	const shared<Image>& getAttachment(std::string_view resource_name) const;
	const shared<RenderPass>& getRenderPass() const { return render_pass; }

public:
	std::vector<unique<Pass>> passes;
	std::vector<Pass*> sorted_passes;
	shared<RenderPass> render_pass = nullptr;
	std::vector<unique<Resource>> resources;
	std::unordered_map<std::string_view, const Resource*> resources_map;
	shared<Fence> previous_frame_finished = nullptr;
	shared<Semaphore> render_finished = nullptr;
	shared<Semaphore> swap_chain_image_available = nullptr;
};