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
		if (auto it = subrenders.find(type_id);  it != subrenders.end() && it->second)
			return (T*)(it->second.get());
		return nullptr;
	}

	template<typename T>
	void addSubrender(const PipelineStage& stage)
	{
		TypeID type_id = TypeInfo<Subrender>::getTypeID<T>();
		stages.emplace(StageIndex(stage, subrenders.size()), type_id);
		subrenders.emplace(type_id, makeUnique<T>(stage));
	}

	template<typename T>
	void removeSubrender()
	{
		TypeID type_id = TypeInfo<Subrender>::getTypeID<T>();
		removeSubrenderStage(type_id);
		subrenders.erase(type_id);
	}

	void clearSubrenders()
	{
		subrenders.clear();
		stages.clear();
	}

	void removeSubrenderStage(const TypeID& id);
	void renderStage(const PipelineStage& pipeline_stage);

private:
	using StageIndex = std::pair<PipelineStage, std::size_t>;
	std::vector<RenderStage> render_stages;
	std::unordered_map<TypeID, unique<Subrender>> subrenders;
	std::multimap<StageIndex, TypeID> stages;
};