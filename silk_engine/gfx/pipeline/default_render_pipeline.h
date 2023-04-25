#pragma once

#include "render_pipeline.h"

class DefaultRenderPipeline : public RenderPipeline
{
public:
	DefaultRenderPipeline();

	void init() override;
};