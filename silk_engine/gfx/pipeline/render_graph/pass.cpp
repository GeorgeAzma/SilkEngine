#include "pass.h"
#include "render_graph.h"

Resource& Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const std::vector<Resource*>& inputs)
{
	Resource& resource = render_graph.addResource(makeUnique<Resource>(name, Resource::Type::ATTACHMENT, *this));
	resource.attachment.format = format;
	resource.attachment.samples = samples;
	outputs.emplace_back(&resource);
	for (Resource* input : inputs)
		this->inputs.emplace_back(input);
	return resource;
}

const shared<RenderPass>& Pass::getRenderPass() const
{
	return render_graph.getRenderPass();
}