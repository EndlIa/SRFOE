#pragma once

#include "Vector.hpp"
#include <algorithm>
#include <cmath>

enum class StyleType {
    NONE,
    CEL,
};
class Stylizer {
public:
    static inline StyleType style = StyleType::NONE;
    static inline void setStyle(StyleType s) { style = s; }
    static inline Vector3f Stylize(const Vector3f &color)
    {
        switch (style) {
            case StyleType::CEL:
                return CelShade(color);
            case StyleType::NONE:
            default:
                return color;
        }
    }
    
private:
    static inline Vector3f CelShade(const Vector3f &color)
    {
        float lum = 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
        lum = std::max(0.0f, std::min(1.0f, lum));
        float intensity = 0.15 + lum;
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

