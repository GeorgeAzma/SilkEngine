#pragma once

#include "shader.h"

class Pipeline : NoCopy
{
private:
	struct StageSpecializationInfo
	{
		VkSpecializationInfo specialization_info{};
		std::vector<uint8_t> constant_data;
		std::vector<VkSpecializationMapEntry> entries;
		void build()
		{
			specialization_info.mapEntryCount = entries.size();
			specialization_info.pMapEntries = entries.data();
			specialization_info.dataSize = constant_data.size();
			specialization_info.pData = constant_data.data();
		}
	};

public:
	struct Constant
	{
		std::string name;
		void* data;
		size_t size;
	};

public:
	virtual ~Pipeline();

	void recreate(const std::vector<Constant>& constants = {});

	virtual void bind() = 0;

	const shared<Shader>& getShader() const { return shader; }
	const VkPipelineLayout& getLayout() const { return layout; }
	operator const VkPipeline& () const { return pipeline; }

private:
	void destroy();

protected:
	virtual void create() = 0;

protected:
	void setShader(const shared<Shader>& shader, const std::vector<Constant>& constants = {});

protected:
	VkPipeline pipeline = nullptr;
	VkPipelineLayout layout = nullptr;
	shared<Shader> shader = nullptr;
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos{};
	std::vector<StageSpecializationInfo> stage_specialization_infos{};
	std::vector<Constant> constants{};
};