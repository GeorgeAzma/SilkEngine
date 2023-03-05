#include "render_pipeline.h"

void RenderPipeline::renderStage(const PipelineStage& pipeline_stage)
{
	for (const auto& [tid, stage, subrender] : subrenders)
		if (stage == pipeline_stage && subrender->enabled)
			subrender->render();
}