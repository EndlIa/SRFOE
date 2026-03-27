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

class ObjectMesh : public Object
{
public:
    ObjectMesh(const std::string& filename, Material* mt = new Material())
    {

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
