#pragma once

#include "Vector.hpp"
#include "Intersection.hpp"

class Light
{
public:
    Light(const Vector3f &p, const Vector3f &i) : position(p), intensity(i) {}
    virtual ~Light() = default;
    virtual float getArea() = 0;
    virtual void Sample(Intersection &pos, float &pdf) = 0;
    Vector3f position;
    Vector3f intensity;
};
