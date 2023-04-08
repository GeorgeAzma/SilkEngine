#include "default_render_pipeline.h"
#include "gfx/devices/physical_device.h"
#include "gfx/window/window.h"
#include "gfx/window/surface.h"
#include "gfx/subrender/mesh_subrender.h"
#include "gfx/subrender/particle_subrender.h"

DefaultRenderPipeline::DefaultRenderPipeline()
{
	shared<RenderPass> render_pass = shared<RenderPass>(new RenderPass({
		{
			{
				{ Image::Format(RenderContext::getPhysicalDevice().getDepthFormat()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, RenderContext::getPhysicalDevice().getMaxSampleCount() },
				{ Image::Format(Window::getActive().getSurface().getFormat().format), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, RenderContext::getPhysicalDevice().getMaxSampleCount() }
			},
			{}
		}
	}));
	addRenderStage(RenderStage(render_pass));
}

void DefaultRenderPipeline::init()
{
	addSubrender<ParticleSubrender>({ 0, 0 });
	addSubrender<MeshSubrender>({ 0, 0 });
}

void DefaultRenderPipeline::update()
{
}
