//
// Created by goksu on 2/25/20.
//
#include "Scene.hpp"
#include "Integrator.hpp"
#pragma once
struct hit_payload
{
    float tNear;
    uint32_t index;
    Vector2f uv;
    Object* hit_obj;
};

class Renderer
{
public:
    Renderer(std::shared_ptr<Integrator> integrator) : integrator(integrator) {}
    void Render(const Scene& scene);
private:
    std::shared_ptr<Integrator> integrator;
};
