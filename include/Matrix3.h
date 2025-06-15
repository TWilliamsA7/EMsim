#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "Vec3.h"
#include <array>

// Works under assumption that correct amount of values has been passed
class Matrix3 {
    public:
        std::array<float, 9> values;

        Matrix3(std::array<float, 9> v) { values = v; };

        inline Matrix3 operator * (const Matrix3 &m) const {
            std::array<float, 9> ret = {};
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    for (int k = 0; k < 3; k++)
                        ret[i * 3 + j] += values[i * 3 + k] * m.values[k * 3 + j];
            return Matrix3(ret);
        }

        inline Vec3f operator * (const Vec3f &v) const {
            float ret[3];
            for (int i = 0; i < 3; i++)
                ret[i] = v.x * values[i * 3] + v.y * values[i * 3 + 1] + v.z * values[i * 3 + 2];
            return Vec3f(ret[0], ret[1], ret[2]);
        }

        std::string toString() {
            std::string ret = "";
            for (int i = 0; i < 3; i++) {
                ret += "(";
                for (int j = 0; j < 3; j++)
                    ret += std::to_string(values[j + i * 3]) + ", ";
                ret += ")\n";
            }
            return ret;
        }
};

#endif //__MATRIX_H__