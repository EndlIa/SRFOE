#pragma once

#include <algorithm>
#include <vector>
#include "Intersection.hpp"
#include "OBJ_Loader.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "Light.hpp"
#include "global.hpp"


class AreaLight : public Light
{
public:
    AreaLight(const objl::Mesh &mesh, const Vector3f &i) : Light(Vector3f(), i)
    {
        area = 0;
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
            triangles.emplace_back(v0, v1, v2);
        }
        for (auto &tri : triangles) {
            area += tri.area;
        }
    }
    float getArea()
    {
        return area;
    }
    void Sample(Intersection &pos, float &pdf)
    {
        if (triangles.empty() || area <= EPSILON) {
            pdf = 0;
            return;
        }
        float p = get_random_float() * area;
        float accum_area = 0;
        for (auto &tri : triangles) {
            accum_area += tri.area;
            if (p <= accum_area) {
                tri.Sample(pos, pdf);
                pdf = 1.0f / area;
                pos.emit = intensity;
                return;
            }
        }
    }
    std::vector<Triangle> triangles;
    float area;
};
