#pragma once


struct AttachmentProps
{
	vk::Format format = vk::Format::eB8G8R8A8Unorm;
	vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal;
	std::optional<vk::ClearValue> clear_value = {};
	vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
	vk::AttachmentLoadOp load_operation = vk::AttachmentLoadOp::eClear;
	vk::AttachmentStoreOp store_operation = vk::AttachmentStoreOp::eStore;
	vk::AttachmentLoadOp stencil_load_operation = vk::AttachmentLoadOp::eDontCare;
	vk::AttachmentStoreOp stencil_store_operation = vk::AttachmentStoreOp::eDontCare;
	vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined;
};
struct Attachment
{
	enum class Type
	{
		COLOR, DEPTH_STENCIL, RESOLVE
	};
	vk::AttachmentDescription description{};
	vk::AttachmentReference reference{};
	vk::ClearValue clear_value{};
	Type type = Type::COLOR;
};

struct Subpass
{
	std::vector<vk::AttachmentReference> input_attachment_references;
	std::vector<Attachment> attachments;
	bool multisampled = false;
};

class RenderPass : NonCopyable
{
public:
	~RenderPass();

	RenderPass& addAttachment(const AttachmentProps& props);
	RenderPass& addSubpass();

	void build();

	void begin(vk::Framebuffer framebuffer, vk::SubpassContents subpass_contents = vk::SubpassContents::eInline);
	void nextSubpass(vk::SubpassContents subpass_contents = vk::SubpassContents::eInline);
	void end();

	operator const vk::RenderPass& () const { return render_pass; }

private:
	vk::RenderPass render_pass;
	std::vector<Subpass> subpasses;
};