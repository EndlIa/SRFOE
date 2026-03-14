#pragma once

#include "BVH.hpp"
#include "Intersection.hpp"
#include "Material.hpp"
#include "OBJ_Loader.hpp"
#include "Object.hpp"
#include "Triangle.hpp"
#include <array>
#include <cassert>
#include <limits>

inline bool rayTriangleIntersect(const Vector3f& v0, const Vector3f& v1,
                                 const Vector3f& v2, const Vector3f& orig,
                                 const Vector3f& dir, float& tnear, float& u,
                                 float& v)
{
    Vector3f edge1 = v1 - v0;
    Vector3f edge2 = v2 - v0;
    Vector3f pvec = crossProduct(dir, edge2);
    float det = dotProduct(edge1, pvec);
    if (det == 0 || det < 0)
        return false;

    Vector3f tvec = orig - v0;
    u = dotProduct(tvec, pvec);
    if (u < 0 || u > det)
        return false;

    Vector3f qvec = crossProduct(tvec, edge1);
    v = dotProduct(dir, qvec);
    if (v < 0 || u + v > det)
        return false;

    float invDet = 1 / det;

    tnear = dotProduct(edge2, qvec) * invDet;
    u *= invDet;
    v *= invDet;

    return true;
}

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

            triangles.emplace_back(v0, v1, v2, mt);
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

    bool intersect(const Ray& ray) { return true; }

    bool intersect(const Ray& ray, float& tnear, uint32_t& index) const
    {
        bool intersect = false;
        for (uint32_t k = 0; k < numTriangles; ++k) {
            const Vector3f& v0 = vertices[vertexIndex[k * 3]];
            const Vector3f& v1 = vertices[vertexIndex[k * 3 + 1]];
            const Vector3f& v2 = vertices[vertexIndex[k * 3 + 2]];
            float t, u, v;
            if (rayTriangleIntersect(v0, v1, v2, ray.origin, ray.direction, t,
                                     u, v) &&
                t < tnear) {
                tnear = t;
                index = k;
                intersect |= true;
            }
        }

        return intersect;
    }

    Bounds3 getBounds() { return bounding_box; }

    void getSurfaceProperties(const Vector3f& P, const Vector3f& I,
                              const uint32_t& index, const Vector2f& uv,
                              Vector3f& N, Vector2f& st) const
    {
        const Vector3f& v0 = vertices[vertexIndex[index * 3]];
        const Vector3f& v1 = vertices[vertexIndex[index * 3 + 1]];
        const Vector3f& v2 = vertices[vertexIndex[index * 3 + 2]];
        Vector3f e0 = normalize(v1 - v0);
        Vector3f e1 = normalize(v2 - v1);
        N = normalize(crossProduct(e0, e1));
        const Vector2f& st0 = stCoordinates[vertexIndex[index * 3]];
        const Vector2f& st1 = stCoordinates[vertexIndex[index * 3 + 1]];
        const Vector2f& st2 = stCoordinates[vertexIndex[index * 3 + 2]];
        st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
    }

    Vector3f evalDiffuseColor(const Vector2f& st) const
    {
        float scale = 5;
        float pattern =
            (fmodf(st.x * scale, 1) > 0.5) ^ (fmodf(st.y * scale, 1) > 0.5);
        return lerp(Vector3f(0.815, 0.235, 0.031),
                    Vector3f(0.937, 0.937, 0.231), pattern);
    }

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
    std::unique_ptr<Vector3f[]> vertices;
    uint32_t numTriangles;
    std::unique_ptr<uint32_t[]> vertexIndex;
    std::unique_ptr<Vector2f[]> stCoordinates;

    std::vector<Triangle> triangles;

    BVHAccel* bvh;
    float area;

    Material* m;
};
