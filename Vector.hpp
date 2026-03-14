
#pragma once

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <limits>

class Vector3f {
public:
    float x, y, z;
    Vector3f() : x(0), y(0), z(0) {}
    Vector3f(float xx) : x(xx), y(xx), z(xx) {}
    Vector3f(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
    Vector3f operator * (const float &r) const { return Vector3f(x * r, y * r, z * r); }
    Vector3f operator / (const float &r) const { return Vector3f(x / r, y / r, z / r); }

    float norm() const {return std::sqrt(x * x + y * y + z * z);}
    Vector3f normalized() const {
        float n = std::sqrt(x * x + y * y + z * z);
        if (n <= std::numeric_limits<float>::epsilon()) {  /// attention for epsilon
            return Vector3f(0.0f);
        }
        return Vector3f(x / n, y / n, z / n);
    }

    float operator * (const Vector3f &v) const { return x * v.x + y * v.y + z * v.z; }
    Vector3f cwiseProduct(const Vector3f &v) const { return Vector3f(x * v.x, y * v.y, z * v.z); }
    Vector3f operator - (const Vector3f &v) const { return Vector3f(x - v.x, y - v.y, z - v.z); }
    Vector3f operator + (const Vector3f &v) const { return Vector3f(x + v.x, y + v.y, z + v.z); }
    Vector3f operator - () const { return Vector3f(-x, -y, -z); }
    Vector3f& operator += (const Vector3f &v) { x += v.x, y += v.y, z += v.z; return *this; }
    friend Vector3f operator * (const float &r, const Vector3f &v)
    { return Vector3f(v.x * r, v.y * r, v.z * r); }
    friend std::ostream & operator << (std::ostream &os, const Vector3f &v)
    { return os << v.x << ", " << v.y << ", " << v.z; }
    static Vector3f Min(const Vector3f &p1, const Vector3f &p2) {
        return Vector3f(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                       std::min(p1.z, p2.z));
    }

    static Vector3f Max(const Vector3f &p1, const Vector3f &p2) {
        return Vector3f(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                       std::max(p1.z, p2.z));
    }
};

inline Vector3f lerp(const Vector3f &a, const Vector3f& b, const float &t)
{ return a * (1 - t) + b * t; }

inline Vector3f normalize(const Vector3f &v)
{
    return v.normalized();
}

inline Vector3f cwiseProduct(const Vector3f &a, const Vector3f &b)
{
    return a.cwiseProduct(b);
}

inline float dotProduct(const Vector3f &a, const Vector3f &b)
{ return a * b; }

inline Vector3f crossProduct(const Vector3f &a, const Vector3f &b)
{
    return Vector3f(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    );
}

class Vector4f {
public:
    float x, y, z, w;
    Vector4f() : x(0), y(0), z(0), w(0) {}
    Vector4f(float xx) : x(xx), y(xx), z(xx), w(xx) {}
    Vector4f(float xx, float yy, float zz, float ww) : x(xx), y(yy), z(zz), w(ww) {}
    Vector4f operator * (const float &r) const { return Vector4f(x * r, y * r, z * r, w * r); }
    Vector4f operator / (const float &r) const { return Vector4f(x / r, y / r, z / r, w / r); }

    float norm() const {return std::sqrt(x * x + y * y + z * z + w * w);}
    Vector4f normalized() const {
        float n = std::sqrt(x * x + y * y + z * z + w * w);
        if (n <= std::numeric_limits<float>::epsilon()) {
            return Vector4f(0.0f);
        }
        return Vector4f(x / n, y / n, z / n, w / n);
    }

    float operator * (const Vector4f &v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    Vector4f cwiseProduct(const Vector4f &v) const { return Vector4f(x * v.x, y * v.y, z * v.z, w * v.w); }
    Vector4f operator - (const Vector4f &v) const { return Vector4f(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vector4f operator + (const Vector4f &v) const { return Vector4f(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vector4f operator - () const { return Vector4f(-x, -y, -z, -w); }
    Vector4f& operator += (const Vector4f &v) { x += v.x, y += v.y, z += v.z, w += v.w; return *this; }
    friend Vector4f operator * (const float &r, const Vector4f &v)
    { return Vector4f(v.x * r, v.y * r, v.z * r, v.w * r); }
    friend std::ostream & operator << (std::ostream &os, const Vector4f &v)
    { return os << v.x << ", " << v.y << ", " << v.z << ", " << v.w; }
    static Vector4f Min(const Vector4f &p1, const Vector4f &p2) {
        return Vector4f(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                       std::min(p1.z, p2.z), std::min(p1.w, p2.w));
    }

    static Vector4f Max(const Vector4f &p1, const Vector4f &p2) {
        return Vector4f(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                       std::max(p1.z, p2.z), std::max(p1.w, p2.w));
    }
};

inline Vector4f lerp(const Vector4f &a, const Vector4f& b, const float &t)
{ return a * (1 - t) + b * t; }

inline Vector4f normalize(const Vector4f &v)
{
    return v.normalized();
}

inline Vector4f cwiseProduct(const Vector4f &a, const Vector4f &b)
{
    return a.cwiseProduct(b);
}

inline float dotProduct(const Vector4f &a, const Vector4f &b)
{ return a * b; }


class Vector2f
{
public:
    Vector2f() : x(0), y(0) {}
    Vector2f(float xx) : x(xx), y(xx) {}
    Vector2f(float xx, float yy) : x(xx), y(yy) {}
    Vector2f operator * (const float &r) const { return Vector2f(x * r, y * r); }
    Vector2f operator + (const Vector2f &v) const { return Vector2f(x + v.x, y + v.y); }
    float x, y;
};


