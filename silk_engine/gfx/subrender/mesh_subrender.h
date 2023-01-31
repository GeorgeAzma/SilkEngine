#pragma once

#include "subrender.h"
#include "gfx/pipeline/pipeline_stage.h"

class MeshSubrender : public Subrender
{
public:
	MeshSubrender(const PipelineStage& pipeline_stage);

	void render() override;
};