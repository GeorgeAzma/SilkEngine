#include "resources.h"
#include "meshes/circle_mesh.h"
#include "meshes/circle_outline_mesh.h"
#include "meshes/rectangle_mesh.h"
#include "meshes/quad_mesh.h"
#include "meshes/cube_mesh.h"
#include "meshes/sphere_mesh.h"
#include "meshes/triangle_mesh.h"
#include "meshes/tetrahedron_mesh.h"
#include "meshes/rounded_rectangle_mesh.h"
#include "gfx/render_context.h"
#include "gfx/window/swap_chain.h"
#include "gfx/descriptors/descriptor_set_layout.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/compute_pipeline.h"
#include "gfx/allocators/command_pool.h"
#include "utils/thread_pool.h"
#include "model.h"
#include "gfx/ui/font.h"

void Resources::init()
{
    // Create directories
    {
        if (!std::filesystem::exists("res/cache/shaders"))
            std::filesystem::create_directories("res/cache/shaders");
        if (!std::filesystem::exists("res/images/screenshots"))
            std::filesystem::create_directories("res/images/screenshots");
    }

    // Images
    {       
        Image::Props image_props{};
        image_props.width = 1;
        image_props.height = 1;
        image_props.sampler_props.min_filter = VK_FILTER_NEAREST;
        image_props.sampler_props.mag_filter = VK_FILTER_NEAREST;
        image_props.sampler_props.anisotropy = 1.0f;
        constexpr u8vec4 white(255);
        auto white_image = makeShared<Image>(image_props);
        white_image->setData(&white);
        white_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        Image::add("White", white_image);
        
        constexpr u8vec4 black(0);
        auto black_image = makeShared<Image>(image_props);
        black_image->setData(&black);
        black_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        Image::add("Black", black_image);
       
        image_props.width = 2;
        image_props.height = 2;
        image_props.sampler_props.u_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        image_props.sampler_props.v_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        constexpr u8vec4 null_data[4] = { { 0, 0, 0, 255 }, { 255, 0, 255, 255 }, { 255, 0, 255, 255 }, { 0, 0, 0, 255 } };
        auto null_image = makeShared<Image>(image_props);
        null_image->setData(null_data);
        null_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        Image::add("Null", null_image);
    }

    // Meshes
    {
        Mesh::add("Triangle", makeShared<Mesh>(TriangleMesh()));
        Mesh::add("Circle", makeShared<Mesh>(CircleMesh()));
        Mesh::add("Circle Outline", makeShared<Mesh>(CircleOutlineMesh()));
        Mesh::add("Rectangle", makeShared<Mesh>(RectangleMesh()));
        Mesh::add("Rounded Rectangle", makeShared<Mesh>(RoundedRectangleMesh()));
        Mesh::add("Quad", makeShared<Mesh>(QuadMesh()));
        Mesh::add("Cube", makeShared<Mesh>(CubeMesh()));
        Mesh::add("Sphere", makeShared<Mesh>(SphereMesh()));
        Mesh::add("Tetrahedron", makeShared<Mesh>(TetrahedronMesh()));
    }

    // Fonts
    {
        Font::add("Arial", makeShared<Font>("arial.ttf"));
    }
}