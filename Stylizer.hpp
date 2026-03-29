#pragma once

#include "Vector.hpp"
#include <algorithm>
#include <cmath>

class Stylizer {
public:
    static inline Vector3f CelShade(const Vector3f &color)
    {
        float lum = 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
        lum = std::max(0.0f, std::min(1.0f, lum));
        double intensity = 0.15 + lum;
        if (intensity > 0.66) intensity = 1.0;
        else if (intensity > 0.33) intensity = 0.66;
        else intensity = 0.33;
        return Vector3f(
            std::min(1.0f, color.x * float(intensity)),
            std::min(1.0f, color.y * float(intensity)),
            std::min(1.0f, color.z * float(intensity))
        );
    }
};

