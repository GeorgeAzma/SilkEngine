#pragma once

#include "render_pipeline.h"

class RenderPass;

class DefaultRenderPipeline : public RenderPipeline
{
public:
	DefaultRenderPipeline();

	void init() override;
	void update() override;

private:
	RenderPass render_pass;
};