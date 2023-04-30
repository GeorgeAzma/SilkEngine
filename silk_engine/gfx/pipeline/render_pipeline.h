#pragma once

class RenderPass;

class RenderPipeline
{
public:
	virtual ~RenderPipeline() = default;

	void render();
	void resize();

public:
	std::vector<shared<RenderPass>> render_passes;
};