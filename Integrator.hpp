#pragma once
#include "Vector.hpp"
#include "Ray.hpp"
#include "Scene.hpp"
class Integrator {
public:
    int maxDepth = 3;
    float RussianRoulette = 0.8;
    virtual ~Integrator() = default;
    virtual Vector3f Li(const Ray &ray, const Scene &scene, int depth) const = 0;
};

class PathTracer : public Integrator {
public:
    Vector3f Li(const Ray &ray, const Scene &scene, int depth) const override;
};
