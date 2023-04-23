#pragma once

#include "subrender.h"
#include "gfx/material.h"

class ParticleSubrender : public Subrender
{
public:
	ParticleSubrender(const PipelineStage& pipeline_stage);

	void render() override;

private:
	shared<Material> material = nullptr;
};