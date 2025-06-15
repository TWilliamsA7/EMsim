#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <vector>
#include <unordered_map>
#include <utility>
#include <cmath>

#include "Vec3.h"
#include <SDL.h>

const float PI = static_cast<float>( 4 * std::atan(1.0));

struct Triangle {
    int a, b, c;

    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}
};

enum class Axis {
    X, Y, Z
};

class Object {
    public:
        Vec3f center;
        bool wireframe;
        SDL_Color color;

        std::vector<Vec3f> vertices;
        std::vector<Triangle> tris;

        // Mechanics
        
        // Stored as units / sec
        Vec3f velocity;
        Vec3f acceleration;

        // Stored in radians
        Vec3f angularVelocity;
        Vec3f angularAcceleration;
        Vec3f rotation;

        // Angle is in degrees
        void updateRotation(Axis axis, float angle);
        void Rotate();
        void Translate();

};

class Tetrahedron : public Object {
    public:
        Tetrahedron(Vec3f center, float radius, SDL_Color color, bool wireframe);
};

class Icosahedron : public Object {
    public:
        Icosahedron(Vec3f center, float radius, SDL_Color color, bool wireframe);
};

struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

class Sphere : public Object {
    public:
        Sphere(Vec3f center, float radius, SDL_Color color, bool wireframe);
    private:
        std::unordered_map<std::pair<int, int>, int, PairHash> midPointIndex;
        float radius;

        int getMidpoint(int i1, int i2);
};

#endif // __OBJECTS_H__