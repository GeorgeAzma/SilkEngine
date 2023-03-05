#pragma once

#include "render_stage.h"
#include "gfx/subrender/subrender.h"
#include "utils/type_info.h"
#include "pipeline_stage.h"

class RenderPipeline
{
public:
	RenderPipeline() = default;
	virtual ~RenderPipeline() = default;

	virtual void init() = 0;
	virtual void update() = 0;

	const std::vector<RenderStage>& getRenderStages() const { return render_stages; }
	std::vector<RenderStage>& getRenderStages() { return render_stages; }

	void addRenderStage(const RenderStage& render_stage)
	{
		render_stages.emplace_back(render_stage);
	}

	template<typename T>
	const T* getSubrender()
	{
		TypeID type_id = TypeInfo<Subrender>::getTypeID<T>();
		for (const auto& [tid, stage, subrender] : subrenders)
			if (type_id == tid)
				return subrender.get();
		return nullptr;
	}

	template<typename T>
	void addSubrender(const PipelineStage& stage)
	{
		TypeID type_id = TypeInfo<Subrender>::getTypeID<T>();
		subrenders.emplace_back(type_id, stage, makeUnique<T>(stage));
	}

	template<typename T>
	void removeSubrender()
	{
		TypeID type_id = TypeInfo<Subrender>::getTypeID<T>();
		for (auto i = subrenders.begin(); i != subrenders.end(); ++i)
			if (std::get<0>(*i) == type_id)
			{
				subrenders.erase(i);
				break;
			}
	}

	void clearSubrenders()
	{
		subrenders.clear();
	}

	void renderStage(const PipelineStage& pipeline_stage);

private:
	std::vector<RenderStage> render_stages;
	std::vector<std::tuple<TypeID, PipelineStage, unique<Subrender>>> subrenders;
};