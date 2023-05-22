#include "resource.h"
#include "pass.h"
#include "gfx/pipeline/render_pass.h"

const shared<Image>& Resource::getAttachment() const
{
	return pass.getRenderPass()->getAttachments()[attachment.index];
}
