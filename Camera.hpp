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
    Camera(float vfov = 40.0f) : fov(vfov)
    {
        CameraM << 1, 0, 0, 278,
                0, 1, 0, 273,
                0, 0, 1, -800,
                0, 0, 0, 1;
    }

    Ray GenerateRay(uint32_t px, uint32_t py, uint32_t width, uint32_t height) const
    {
        Vector3f localDir = localDirection(px, py, width, height);
        Vector4f worldDir = CameraM * Vector4f(localDir.x, localDir.y, localDir.z, 0);
        return Ray(CameraM.col(3).to3(), worldDir.to3());
    }

    Vector3f localDirection(uint32_t px, uint32_t py, uint32_t width, uint32_t height) const
    {
        float scale = std::tan(Deg2Rad(fov * 0.5f));
        float imageAspectRatio = width / static_cast<float>(height);
        float x = (2 * (px + 0.5f) / static_cast<float>(width) - 1) * imageAspectRatio * scale;
        float y = (1 - 2 * (py + 0.5f) / static_cast<float>(height)) * scale;
        return normalize(Vector3f(-x, y, 1));
    }
};