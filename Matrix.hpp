#pragma once

#include "Vector.hpp"
#include <cstring>

class Matrix4f {
public:
    float m[16];  //col major
    Matrix4f() {
        std::memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }
    Matrix4f(const float *data) {
        std::memcpy(m, data, sizeof(m));
    }
    Matrix4f(const Vector4f &col0, const Vector4f &col1,
             const Vector4f &col2, const Vector4f &col3) {
        m[0] = col0.x;   m[4] = col1.x;   m[8] = col2.x;   m[12] = col3.x;
        m[1] = col0.y;   m[5] = col1.y;   m[9] = col2.y;   m[13] = col3.y;
        m[2] = col0.z;   m[6] = col1.z;   m[10] = col2.z;  m[14] = col3.z;
        m[3] = col0.w;   m[7] = col1.w;   m[11] = col2.w;  m[15] = col3.w;
    }
    Matrix4f operator * (const Matrix4f &other) const {
        Matrix4f result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += (*this)(i, k) * other(k, j);
                }
                result(i, j) = sum;
            }
        }
        return result;
    }

    Vector4f operator * (const Vector4f &v) const {
        Vector4f result;
        result.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
        result.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
        result.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
        result.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
        return result;
    }

    float& operator () (int row, int col) {
        return m[col * 4 + row];
    }

    float operator () (int row, int col) const {
        return m[col * 4 + row];
    }

    Vector4f col(int i) const {
        return Vector4f(m[i * 4], m[i * 4 + 1], m[i * 4 + 2], m[i * 4 + 3]);
    }

    Vector4f row(int i) const {
        return Vector4f(m[i], m[i + 4], m[i + 8], m[i + 12]);
    }

    class MatrixInitializer {
        Matrix4f& m;
        int i = 0;
        void set(float v) { if(i < 16) m.m[(i%4)*4 + i/4] = v; i++; }
    public:
        MatrixInitializer(Matrix4f& mat, float v) : m(mat) { set(v); }
        MatrixInitializer& operator,(float v) { set(v); return *this; }
    };
    MatrixInitializer operator<<(float v) { return MatrixInitializer(*this, v); }
};
