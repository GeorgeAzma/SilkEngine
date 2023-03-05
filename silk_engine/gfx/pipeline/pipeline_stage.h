#pragma once

struct PipelineStage
{
	uint32_t render_pass;
	uint32_t subpass;
	bool operator==(const PipelineStage& other) const { return render_pass == other.render_pass && subpass == other.subpass; }
};