#pragma once

#include <cstdint>
#include <cmath>
#include "Matrix.hpp"
#include "Ray.hpp"

class Camera
{
public:
    Matrix4f CameraM;
    float fov;
    Camera(float vfov = 56.2f) : fov(vfov)
    {
        CameraM << 0.5720190405845642,    0.44133055210113525, -0.6913909912109375,  -4.769540309906006,
     3.508610575408966e-07, 0.8429126143455505,  0.5380504727363586,   4.82857608795166,
     0.820240318775177,    -0.3077753782272339,  0.48216190934181213, 3.924150228500366,
     0.0,                   0.0,                  0.0,                  1.0;
    }

    Ray GenerateRay(uint32_t px, uint32_t py, uint32_t width, uint32_t height) const
    {
        Vector3f localDir = localDirection(px, py, width, height);
        Vector4f worldDir = CameraM * Vector4f(localDir.x, localDir.y, localDir.z, 0);
        return Ray(CameraM.col(3).to3(), worldDir.to3().normalized());
    }

    Vector3f localDirection(uint32_t px, uint32_t py, uint32_t width, uint32_t height) const
    {
        float scale = std::tan(Deg2Rad(fov * 0.5f));
        float imageAspectRatio = width / static_cast<float>(height);
        float x = (2 * (px + 0.5f) / static_cast<float>(width) - 1) * imageAspectRatio * scale;
        float y = (1 - 2 * (py + 0.5f) / static_cast<float>(height)) * scale;
        return normalize(Vector3f(x, y, -1));
    }
};