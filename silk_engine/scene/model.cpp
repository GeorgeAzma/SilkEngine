#include "model.h"
#include "raw_model.h"
#include "meshes/mesh.h"
#include "gfx/images/image.h"

Model::Model(const fs::path& file)
{
    *this = RawModel(file);
}

RawModel Model::load(const fs::path& file)
{
    return RawModel(file);
}

Model::Model(const RawModel& raw_model)
{
    meshes.resize(raw_model.meshes.size());

    for(size_t i = 0; i < meshes.size(); ++i)
        meshes[i] = makeShared<Mesh>(*raw_model.meshes[i]);
    file = raw_model.file;

    Image::Props image_props{};
    auto white = Image::get("White");
    auto black = Image::get("Black");
    
    std::unordered_map<size_t, shared<Image>> bitmap_images;

    images.clear();
    images.reserve(raw_model.material_data.size());
    for (size_t i = 0; i < raw_model.material_data.size(); ++i) //Material data for each mesh
    {
        const auto& md = raw_model.material_data[i];
        images.emplace_back();

        if (md.diffuse_map)
        {
            if (auto bi = bitmap_images.find((size_t)md.diffuse_map); bi != bitmap_images.end())
                images.back().emplace_back(bi->second);
            else
            {
                image_props.width = md.diffuse_map->width;
                image_props.height = md.diffuse_map->height;
                image_props.format = Image::getFormatFromChannelCount(md.diffuse_map->channels);
                images.back().emplace_back(makeShared<Image>(image_props));
                images.back().back()->setData(md.diffuse_map->pixels.data());
                bitmap_images.insert_or_assign((size_t)md.diffuse_map, images.back().back());
            }
        }
        else images.back().emplace_back(white);

        if (md.normal_map)
        {
            if (auto bi = bitmap_images.find((size_t)md.normal_map); bi != bitmap_images.end())
                images.back().emplace_back(bi->second);
            else
            {
                image_props.width = md.normal_map->width;
                image_props.height = md.normal_map->height;
                image_props.format = Image::getFormatFromChannelCount(md.normal_map->channels);
                images.back().emplace_back(makeShared<Image>(image_props));
                images.back().back()->setData(md.diffuse_map->pixels.data());
                bitmap_images.emplace((size_t)md.normal_map, images.back().back());
            }
        }
        else images.back().emplace_back(black);

        if (md.height_map)
        {
            if (auto bi = bitmap_images.find((size_t)md.height_map); bi != bitmap_images.end())
                images.back().emplace_back(bi->second);
            else
            {
                image_props.width = md.height_map->width;
                image_props.height = md.height_map->height;
                image_props.format = Image::getFormatFromChannelCount(md.height_map->channels);
                images.back().emplace_back(makeShared<Image>(image_props));
                images.back().back()->setData(md.diffuse_map->pixels.data());
                bitmap_images.emplace((size_t)md.height_map, images.back().back());
            }
        }
        else images.back().emplace_back(black);

        if (md.ao_map)
        {
            if (auto bi = bitmap_images.find((size_t)md.ao_map); bi != bitmap_images.end())
                images.back().emplace_back(bi->second);
            else
            {
                image_props.width = md.ao_map->width;
                image_props.height = md.ao_map->height;
                image_props.format = Image::getFormatFromChannelCount(md.ao_map->channels);
                images.back().emplace_back(makeShared<Image>(image_props));
                images.back().back()->setData(md.diffuse_map->pixels.data());
                bitmap_images.emplace((size_t)md.ao_map, images.back().back());
            }
        }
        else images.back().emplace_back(white);

        if (md.specular_map)
        {
            if (auto bi = bitmap_images.find((size_t)md.specular_map); bi != bitmap_images.end())
                images.back().emplace_back(bi->second);
            else
            {
                image_props.width = md.specular_map->width;
                image_props.height = md.specular_map->height;
                image_props.format = Image::getFormatFromChannelCount(md.specular_map->channels);
                images.back().emplace_back(makeShared<Image>(image_props));
                images.back().back()->setData(md.diffuse_map->pixels.data());
                bitmap_images.emplace((size_t)md.specular_map, images.back().back());
            }
        }
        else images.back().emplace_back(black);

        if (md.emissive_map)
        {
            if (auto bi = bitmap_images.find((size_t)md.emissive_map); bi != bitmap_images.end())
                images.back().emplace_back(bi->second);
            else
            {
                image_props.width = md.emissive_map->width;
                image_props.height = md.emissive_map->height;
                image_props.format = Image::getFormatFromChannelCount(md.emissive_map->channels);
                images.back().emplace_back(makeShared<Image>(image_props));
                images.back().back()->setData(md.diffuse_map->pixels.data());
                bitmap_images.emplace((size_t)md.emissive_map, images.back().back());
            }
        }
        else images.back().emplace_back(black);
    }
}

const std::vector<shared<Mesh>>& Model::getMeshes() const
{
    return meshes;
}

const std::vector<std::vector<shared<Image>>>& Model::getImages() const
{
    return images;
}