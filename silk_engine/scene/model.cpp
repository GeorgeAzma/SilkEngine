#include "model.h"
#include "raw_model.h"
#include "meshes/mesh.h"
#include "silk_engine/gfx/images/image.h"

Model::Model(const fs::path& file)
    : Model(RawModel(file))
{}

RawModel Model::load(const fs::path& file)
{
    return RawModel(file);
}

Model::Model(const RawModel& raw_model)
{
    file = raw_model.file;
    mesh = makeShared<Mesh>(raw_model.mesh);
    images.resize(raw_model.images.size());
    for (size_t i = 0; i < raw_model.images.size(); ++i)
    {
        images[i] = makeShared<Image>(raw_model.images[i]);
        images[i]->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    materials = raw_model.materials;
    nodes = raw_model.nodes;
}