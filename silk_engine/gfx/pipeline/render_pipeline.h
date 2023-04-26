#pragma once

#include "gfx/subrender/subrender.h"
#include "utils/type_info.h"
#include "gfx/pipeline/render_pass.h"

struct PipelineStage
{
	uint32_t render_pass;
	uint32_t subpass;
	bool operator==(const PipelineStage& other) const { return render_pass == other.render_pass && subpass == other.subpass; }
};

class RenderPipeline
{
public:
	RenderPipeline() = default;
	virtual ~RenderPipeline() = default;

	virtual void init() = 0;

	void render();
	void resize();

	const std::vector<shared<RenderPass>>& getRenderPasses() const { return render_passes; }
	std::vector<shared<RenderPass>>& getRenderPasses() { return render_passes; }

	void addRenderPass(const shared<RenderPass>& render_pass)
	{
		render_passes.emplace_back(render_pass);
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

private:
	std::vector<shared<RenderPass>> render_passes;
	std::vector<std::tuple<TypeID, PipelineStage, unique<Subrender>>> subrenders;
};