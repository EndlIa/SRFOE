#pragma once

#include "BVH.hpp"
#include "Intersection.hpp"
#include "Material.hpp"
#include "OBJ_Loader.hpp"
#include "Object.hpp"
#include "Triangle.hpp"
#include "Texture.hpp"
#include <cassert>
#include <limits>

class MeshTriangle : public Object
{
public:
    MeshTriangle(const std::string& filename, Material* mt = new Material())
    {
        objl::Loader loader;
        loader.LoadFile(filename);
        area = 0;
        m = mt;
        bvh = nullptr;
        assert(loader.LoadedMeshes.size() == 1);
        auto mesh = loader.LoadedMeshes[0];

        Vector3f min_vert = Vector3f{std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity()};
        Vector3f max_vert = Vector3f{-std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity()};
        bool has_valid_triangle = false;
        for (size_t i = 0; i + 2 < mesh.Indices.size(); i += 3) {
            const unsigned int idx0 = mesh.Indices[i];
            const unsigned int idx1 = mesh.Indices[i + 1];
            const unsigned int idx2 = mesh.Indices[i + 2];

            if (idx0 >= mesh.Vertices.size() || idx1 >= mesh.Vertices.size() ||
                idx2 >= mesh.Vertices.size()) {
                continue;
            }

            const auto v0 = Vector3f(mesh.Vertices[idx0].Position.X,
                                     mesh.Vertices[idx0].Position.Y,
                                     mesh.Vertices[idx0].Position.Z);
            const auto v1 = Vector3f(mesh.Vertices[idx1].Position.X,
                                     mesh.Vertices[idx1].Position.Y,
                                     mesh.Vertices[idx1].Position.Z);
            const auto v2 = Vector3f(mesh.Vertices[idx2].Position.X,
                                     mesh.Vertices[idx2].Position.Y,
                                     mesh.Vertices[idx2].Position.Z);

            const auto vt0 = Vector3f(mesh.Vertices[idx0].TextureCoordinate.X,
                                     mesh.Vertices[idx0].TextureCoordinate.Y, 0.0f);
            const auto vt1 = Vector3f(mesh.Vertices[idx1].TextureCoordinate.X,
                                     mesh.Vertices[idx1].TextureCoordinate.Y, 0.0f);
            const auto vt2 = Vector3f(mesh.Vertices[idx2].TextureCoordinate.X,
                                     mesh.Vertices[idx2].TextureCoordinate.Y, 0.0f);

            min_vert = Vector3f(std::min(min_vert.x, v0.x),
                                std::min(min_vert.y, v0.y),
                                std::min(min_vert.z, v0.z));
            min_vert = Vector3f(std::min(min_vert.x, v1.x),
                                std::min(min_vert.y, v1.y),
                                std::min(min_vert.z, v1.z));
            min_vert = Vector3f(std::min(min_vert.x, v2.x),
                                std::min(min_vert.y, v2.y),
                                std::min(min_vert.z, v2.z));

            max_vert = Vector3f(std::max(max_vert.x, v0.x),
                                std::max(max_vert.y, v0.y),
                                std::max(max_vert.z, v0.z));
            max_vert = Vector3f(std::max(max_vert.x, v1.x),
                                std::max(max_vert.y, v1.y),
                                std::max(max_vert.z, v1.z));
            max_vert = Vector3f(std::max(max_vert.x, v2.x),
                                std::max(max_vert.y, v2.y),
                                std::max(max_vert.z, v2.z));

            triangles.emplace_back(v0, v1, v2, vt0, vt1, vt2, mt);
            has_valid_triangle = true;
        }

        if (has_valid_triangle) {
            bounding_box = Bounds3(min_vert, max_vert);
        } else {
            bounding_box = Bounds3();
        }

        std::vector<Object*> ptrs;
        for (auto& tri : triangles) {
            ptrs.push_back(&tri);
            area += tri.area;
        }
        if (!ptrs.empty()) {
            bvh = new BVHAccel(ptrs);
        }
    }

    Bounds3 getBounds() { return bounding_box; }



    Intersection getIntersection(Ray ray)
    {
        Intersection intersec;

        if (bvh) {
            intersec = bvh->Intersect(ray);
        }

        return intersec;
    }

    void Sample(Intersection& pos, float& pdf)
    {
        if (!bvh) {
            pdf = 0;
            return;
        }
        bvh->Sample(pos, pdf);
        pos.emit = m->getEmission();
    }
    float getArea() { return area; }
    bool hasEmit() { return m->hasEmission(); }

    Bounds3 bounding_box;
    std::vector<Triangle> triangles;

    BVHAccel* bvh;
    float area;

    Material* m;
};
