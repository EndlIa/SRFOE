
#include "Scene.hpp"
#include "Integrator.hpp"
#pragma once

class Renderer
{
public:
    Renderer(std::shared_ptr<Integrator> integrator) : integrator(integrator) {}
    void Render(const Scene& scene);
    void SaveImage(const std::string& filename, float gamma = 0.6f) const;
    const std::vector<Vector3f>& GetFramebuffer() const { return framebuffer; }
private:
    std::shared_ptr<Integrator> integrator;
    std::vector<Vector3f> framebuffer;
    int width = 0, height = 0;
};
