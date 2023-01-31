#include "bezier_mesh.h"
#include "line_mesh.h"

BezierMesh::BezierMesh(float x1, float y1, float px, float py, float x2, float y2, uint32_t resolution, float width, uint32_t edge_resolution, bool tile_UVs)
{
    SK_ASSERT(resolution > 2, "Resolution of bezier curve must be more than 2, if it's 2 or 1 use line mesh or circle mesh accordingly");
    std::vector<vec2> points(resolution);
    points.front() = { x1, y1 };
    vec2 p = { px, py };
    points.back() = { x2, y2 };

    size_t joints = points.size() - 1;
    for (size_t i = 1; i < joints; ++i)
    {
        float t = float(i) / joints;
        points[i] = math::lerp(math::lerp(points.front(), p, t), math::lerp(points.back(), p, t), t);
    }

    LineMesh line_mesh(points, width, edge_resolution, tile_UVs);
    vertices = std::move(line_mesh.vertices);
    indices = std::move(line_mesh.indices);
}

BezierMesh::BezierMesh(float x1, float y1, float px1, float py1, float px2, float py2, float x2, float y2, uint32_t resolution, float width, uint32_t edge_resolution, bool tile_UVs)
{
    SK_ASSERT(resolution > 3, "Resolution of cubic bezier curve must be more than 3, if it's 3, 2 or 1 use quadratic bezier mesh, line mesh or circle mesh accordingly");
    std::vector<vec2> points(resolution);
    points.front() = { x1, y1 };
    vec2 p1 = { px1, py1 };
    vec2 p2 = { px2, py2 };
    points.back() = { x2, y2 };

    size_t joints = points.size() - 1;
    for (size_t i = 1; i < joints; ++i)
    {
        float t = float(i) / joints;
        points[i] =
            (1 - t) * (1 - t) * (1 - t) * points.front()
            + 3 * (1 - t) * (1 - t) * t * p1
            + 3 * (1 - t) * t * t * p2
            + t * t * t * points.back();
    }

    LineMesh line_mesh(points, width, edge_resolution, tile_UVs);
    vertices = std::move(line_mesh.vertices);
    indices = std::move(line_mesh.indices);
}