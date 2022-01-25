#pragma once

#include "core/application.h"
#include "core/log.h"
#include "core/platform.h"
#include "core/event.h"
#include "core/layer.h"
#include "core/time.h"
#include "core/layer_stack.h"

#include "core/input/mouse_buttons.h"
#include "core/input/joystick_buttons.h"
#include "core/input/keys.h"
#include "core/input/input.h"

#include "utils/debug_timer.h"
#include "utils/fixed_update.h"
#include "utils/delta.h"
#include "utils/file_utils.h"
#include "utils/math.h"
#include "utils/RNG.h"
#include "utils/timers.h"
#include "utils/detected.h"
#include "utils/thread_pool.h"

#include "gfx/graphics.h"
#include "gfx/enums.h"

#include "gfx/allocators/command_pool.h"
#include "gfx/allocators/query_pool.h"

#include "gfx/pipeline/shader.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/render_pass.h"

#include "gfx/descriptors/descriptor_pool.h"
#include "gfx/descriptors/descriptor_set.h"
#include "gfx/descriptors/descriptor_set_layout.h"

#include "gfx/window/window.h"

#include "gfx/images/image.h"
#include "gfx/images/image_view.h"
#include "gfx/images/image_array.h"
#include "gfx/images/sampler.h"

#include "gfx/buffers/buffer.h"
#include "gfx/buffers/vertex_buffer.h"
#include "gfx/buffers/index_buffer.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/buffers/framebuffer.h"
#include "gfx/buffers/staging_buffer.h"
#include "gfx/buffers/buffer_layout.h"
#include "gfx/buffers/uniform_buffer.h"
#include "gfx/buffers/storage_buffer.h"
#include "gfx/buffers/indirect_buffer.h"
#include "gfx/buffers/vertex_array.h"

#include "scene/vertex.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scene/components.h"
#include "scene/resources.h"

#include "scene/camera/camera_controller.h"
#include "scene/camera/camera.h"

#include "scene/meshes/mesh.h"
#include "scene/meshes/circle_mesh.h"
#include "scene/meshes/rectangle_mesh.h"
