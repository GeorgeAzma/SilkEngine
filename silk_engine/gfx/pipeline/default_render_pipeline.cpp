#include "default_render_pipeline.h"
#include "gfx/devices/physical_device.h"
#include "gfx/window/window.h"
#include "gfx/window/surface.h"
#include "gfx/subrender/mesh_subrender.h"
#include "gfx/subrender/particle_subrender.h"
#include "gfx/render_context.h"
#include "render_pass.h"

DefaultRenderPipeline::DefaultRenderPipeline()
{
	shared<RenderPass> render_pass = shared<RenderPass>(new RenderPass({
		   {
			   {
				   { Image::Format(RenderContext::getPhysicalDevice().getDepthFormat()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, RenderContext::getPhysicalDevice().getMaxSampleCount() },
				   { Image::Format(Window::getActive().getSurface().getFormat().format), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, RenderContext::getPhysicalDevice().getMaxSampleCount() }
			   }, {}
		   }
		}));
	render_pass->addSubrender<ParticleSubrender>(0);
	render_pass->addSubrender<MeshSubrender>(0);
	render_passes.emplace_back(render_pass);
}