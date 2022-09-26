#include "render_pipeline.h"

void RenderPipeline::removeSubrenderStage(const TypeID& id)
{
	for (auto it = stages.begin(); it != stages.end();) 
	{
		if (it->second == id)
			it = stages.erase(it);
		else ++it;
	}
}

void RenderPipeline::renderStage(const PipelineStage& pipeline_stage)
{
	for (const auto& [stage_index, type_id] : stages)
	{
		if (pipeline_stage != stage_index.first)
			continue;

		if (auto& subrender = subrenders[type_id])
			if (subrender->enabled)
				subrender->render();
	}
}