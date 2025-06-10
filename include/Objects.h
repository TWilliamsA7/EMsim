#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <vector>
#include <cmath>

#include "Vec3.h"

struct Triangle {
    Vec3f a, b, c;

    Triangle(Vec3f a, Vec3f b, Vec3f c) : a(a), b(b), c(c) {}
};

enum class Axis {
    X, Y, Z
};

class Object {
    public:
        Vec3f center;

        std::vector<Vec3f> vertices;
        std::vector<Triangle> tris;

        // Angle is in degrees
        void Rotate(Axis axis, float angle);

};

class Tetrahedron : public Object {
    public:
        Tetrahedron(Vec3f center, float radius);

};

#endif // __OBJECTS_H__