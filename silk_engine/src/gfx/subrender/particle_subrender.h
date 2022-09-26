#pragma once

#include "subrender.h"
#include "gfx/pipeline/pipeline_stage.h"
#include "gfx/pipeline/graphics_pipeline.h"

class ParticleSubrender : public Subrender
{
public:
	ParticleSubrender(const PipelineStage& pipeline_stage);

	void render() override;

private:
	GraphicsPipeline pipeline;
};