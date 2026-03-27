#pragma once

#include "Intersection.hpp"
#include "Material.hpp"
#include "Object.hpp"

class Triangle : public Object
{
public:
    Vector3f v0, v1, v2; // vertices A, B ,C , counter-clockwise order
    Vector3f e1, e2;     // 2 edges v1-v0, v2-v0;
    Vector3f t0, t1, t2; // texture coords
    Vector3f normal;
    float area;
    Material* m;

    Triangle(Vector3f _v0, Vector3f _v1, Vector3f _v2,
             Vector3f _t0, Vector3f _t1, Vector3f _t2,
             Material* _m = nullptr)
        : v0(_v0), v1(_v1), v2(_v2), t0(_t0), t1(_t1), t2(_t2), m(_m)
    {
        e1 = v1 - v0;
        e2 = v2 - v0;
        normal = normalize(crossProduct(e1, e2));
        area = crossProduct(e1, e2).norm()*0.5f;
    }
    Intersection getIntersection(Ray ray) override;

    Bounds3 getBounds(){
        return Union(Bounds3(v0, v1), v2);
    }
    void Sample(Intersection &pos, float &pdf){
        float x = std::sqrt(get_random_float()), y = get_random_float();
        pos.coords = v0 * (1.0f - x) + v1 * (x * (1.0f - y)) + v2 * (x * y);
        pos.normal = this->normal;
        pdf = 1.0f / area;
    }
    float getArea(){
        return area;
    }
    bool hasEmit(){
        return m->hasEmission();
    }
};

inline Intersection Triangle::getIntersection(Ray ray)
{
    Intersection inter;

    if (dotProduct(ray.direction, normal) > 0)
        return inter;
    double u, v, t_tmp = 0;
    Vector3f pvec = crossProduct(ray.direction, e2);
    double det = dotProduct(e1, pvec);
    if (fabs(det) < EPSILON)
        return inter;

    double det_inv = 1. / det;
    Vector3f tvec = ray.origin - v0;
    u = dotProduct(tvec, pvec) * det_inv;
    if (u < 0 || u > 1)
        return inter;
    Vector3f qvec = crossProduct(tvec, e1);
    v = dotProduct(ray.direction, qvec) * det_inv;
    if (v < 0 || u + v > 1)
        return inter;
    t_tmp = dotProduct(e2, qvec) * det_inv;

    // TODO find ray triangle intersection
    if (t_tmp > EPSILON && t_tmp < inter.distance) {
        inter.happened = true;
        inter.coords = ray(t_tmp);
        inter.normal = normal;
        inter.distance = t_tmp;
        inter.obj = this;
        inter.m = m;
        // interpolate texture coordinates (u,v are barycentric for v1 and v2)
        inter.tcoords =  (1.0f - u - v) * t0 + u * t1 + v * t2;
    }
    return inter;
}


