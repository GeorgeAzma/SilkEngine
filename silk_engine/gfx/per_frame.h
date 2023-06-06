#pragma once

#include "render_context.h"

template <typename T>
class PerFrame : public T
{
    size_t frame_index = 0;
public:
    template<typename... Args>
    PerFrame(Args&&... args)
    {
        resources.resize(RenderContext::MAX_FRAMES);
        for (size_t i = 0; i < RenderContext::MAX_FRAMES; ++i)
            resources[i] = makeShared<T>(std::forward<Args>(args)...);
    }

    T& get() { return *resources[frame_index]; }
    const T& get() const { return *resources[frame_index]; }
    T& operator*() { return *resources[frame_index]; }
    T* operator->() { return resources[frame_index].get(); }
    const T& operator*() const { return *resources[frame_index]; }
    const T* operator->() const { return resources[frame_index].get(); }

private:
    std::vector<shared<T>> resources;
};