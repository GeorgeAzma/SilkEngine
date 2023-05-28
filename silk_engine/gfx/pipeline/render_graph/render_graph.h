#pragma once

#include "resource.h"
#include "pass.h"

class RenderPass;
class SwapChain;
class Semaphore;
class CommandBuffer;

class RenderGraph
{
public:
	~RenderGraph();

	Pass& addPass(const char* name) 
	{
		passes.emplace_back(makeUnique<Pass>(name, Pass::Type::RENDER, *this));
		passes_map.emplace(name, passes.back().get());
		return *passes.back(); 
	}
	Pass& addComputePass(const char* name)
	{
		passes.emplace_back(makeUnique<Pass>(name, Pass::Type::COMPUTE, *this));
		passes_map.emplace(name, passes.back().get());
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
	const Pass& getPass(std::string_view name) const { return *passes_map.at(name); }
	const RenderPass& getRenderPass() const { return *render_pass; }

private:
	std::vector<unique<Pass>> passes;
	std::unordered_map<std::string_view, const Pass*> passes_map;
	std::vector<Pass*> sorted_passes;
	shared<RenderPass> render_pass = nullptr;
	std::vector<unique<Resource>> resources;
	std::unordered_map<std::string_view, const Resource*> resources_map;
	shared<CommandBuffer> command_buffer = nullptr;
	shared<Semaphore> render_finished = nullptr;
	shared<Semaphore> swap_chain_image_available = nullptr;
};