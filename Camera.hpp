#pragma once

#include <cstdint>
#include <cmath>
#include "Ray.hpp"

class Camera
{
public:
    Camera(const Vector3f& pos = Vector3f(278, 273, -800), float vfov = 40.0f)
        : position(pos), fov(vfov)
    {}

    Ray GenerateRay(uint32_t pixelX, uint32_t pixelY, uint32_t width, uint32_t height) const
    {
        Vector3f dir = GenerateDirection(pixelX, pixelY, width, height);
        return Ray(position, dir);
    }

    Vector3f GenerateDirection(uint32_t pixelX, uint32_t pixelY, uint32_t width, uint32_t height) const
    {
        float scale = std::tan(Deg2Rad(fov * 0.5f));
        float imageAspectRatio = width / static_cast<float>(height);
        float x = (2 * (pixelX + 0.5f) / static_cast<float>(width) - 1) * imageAspectRatio * scale;
        float y = (1 - 2 * (pixelY + 0.5f) / static_cast<float>(height)) * scale;
        return normalize(Vector3f(-x, y, 1));
    }

    Vector3f position;
    float fov;

private:
    static float Deg2Rad(float deg)
    {
        constexpr float pi = 3.14159265358979323846f;
        return deg * pi / 180.0f;
    }
};