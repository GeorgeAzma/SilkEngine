#pragma once

#include "buffers/framebuffer.h"
#include "buffers/command_buffer.h"
#include "image_view.h"
#include "image.h"
#include "core/event.h"
#include "render_pass.h"

class SwapChain : NonCopyable
{
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    friend class PhysicalDevice;
public:
    SwapChain(const std::optional<VkSwapchainKHR>& old_swap_chain = {});
    ~SwapChain();

    operator const VkSwapchainKHR& () const { return swap_chain; }

    const VkSurfaceFormatKHR& getSurfaceFormat() const { return surface_format; }
    const VkExtent2D& getExtent() const { return extent; }
    const std::vector<Framebuffer*>& getFramebuffers() const { return framebuffers; }
    const std::vector<VkImage>& getImages() const { return images; }
    const VkRenderPass& getRenderPass() const { return *render_pass; }
    uint32_t getImageIndex() const { return image_index; }
    VkSampleCountFlagBits getSampleCount() const { return sample_count; }
    
    void recreate();

    void createFramebuffers();
    void acquireNextImage();
    void present();

    void beginFrame();
    void endFrame();
    
private:
    void create(const std::optional<VkSwapchainKHR>& old_swap_chain = {});
    void destroy();

    void chooseSwapChainSurfaceFormat();
    void chooseSwapChainPresentMode();
    void chooseSwapChainExtent();
    int rateSwapChainPresentMode(VkPresentModeKHR present_mode) const;
    int rateSwapChainSurfaceFormat(VkSurfaceFormatKHR format) const;

    void onWindowResize(const WindowResizeEvent& e);

private:
    VkSwapchainKHR swap_chain;
    std::vector<VkImage> images;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
    std::vector<ImageView*> image_views;
    std::vector<Framebuffer*> framebuffers;
    std::vector<VkFence> in_flight_fences = std::vector<VkFence>(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSemaphore> image_available_semaphores = std::vector<VkSemaphore>(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSemaphore> render_finished_semaphores = std::vector<VkSemaphore>(MAX_FRAMES_IN_FLIGHT);
    uint32_t current_frame = 0;
    CommandBuffer* command_buffer = nullptr;
    std::vector<VkFence> images_in_flight = {};
    uint32_t image_index = 0;
    Image* msaa_image = nullptr;
    Image* depth = nullptr;
    VkFormat depth_format;
    uint32_t image_count = 0;
    RenderPass* render_pass = nullptr;
    VkSampleCountFlagBits sample_count;
};