#include "Objects.h"

#include <iostream>
#include <algorithm>

// Needs to affect the triangles too 
void Object::Rotate(Axis axis, float angle) {

    // Convert degrees to radians
    const float PI = static_cast<float>( 4 * std::atan(1.0));
    angle *= (PI / 180.0f);
    float s = std::sin(angle);
    float c = std::cos(angle);

    float rotMatrix[3][3];

    switch (axis) {
        // X-axis
        case Axis::X:

            rotMatrix[0][0] = 1; rotMatrix[0][1] = 0; rotMatrix[0][2] = 0;
            rotMatrix[1][0] = 0; rotMatrix[1][1] = c; rotMatrix[1][2] = -s;
            rotMatrix[2][0] = 0; rotMatrix[2][1] = s; rotMatrix[2][2] = c;
            break;

        // Y-axis
        case Axis::Y:

            rotMatrix[0][0] = c; rotMatrix[0][1] = 0; rotMatrix[0][2] = s;
            rotMatrix[1][0] = 0; rotMatrix[1][1] = 1; rotMatrix[1][2] = 0;
            rotMatrix[2][0] = -s; rotMatrix[2][1] = 0; rotMatrix[2][2] = c;
            break;

        // Z-axis
        case Axis::Z:

            rotMatrix[0][0] = c; rotMatrix[0][1] = -s; rotMatrix[0][2] = 0;
            rotMatrix[1][0] = s; rotMatrix[1][1] = c; rotMatrix[1][2] = 0;
            rotMatrix[2][0] = 0; rotMatrix[2][1] = 0; rotMatrix[2][2] = 1;
            break;
    }

    for (Vec3f& vert : vertices) {

        // Move to origin for rotation
        vert = vert - center;
        
        float ox = vert.x;
        float oy = vert.y;
        float oz = vert.z;
        // Multiply by the rotation matix
        vert.x = ox * rotMatrix[0][0] + oy * rotMatrix[1][0] + oz * rotMatrix[2][0];
        vert.y = ox * rotMatrix[0][1] + oy * rotMatrix[1][1] + oz * rotMatrix[2][1];
        vert.z = ox * rotMatrix[0][2] + oy * rotMatrix[1][2] + oz * rotMatrix[2][2];

        // Return to center point
        vert = vert + center;
    }
}

Tetrahedron::Tetrahedron(Vec3f center, float radius) {
    // Computed value for regular tetrahedron
    float ang = 0.33981f;
    float b = radius * std::sin(ang);
    float c = radius * std::cos(ang);
    
    this->center = center;
    
    vertices.push_back(center + Vec3f(0, 0, radius));
    vertices.push_back(center + Vec3f(c, 0, b));
    vertices.push_back(center + Vec3f(-c, 0, b));
    vertices.push_back(center + Vec3f(0, c, b));
    
    tris.push_back(Triangle(0, 1, 2));
    tris.push_back(Triangle(0, 1, 3));
    tris.push_back(Triangle(0, 2, 3));
    tris.push_back(Triangle(1, 2, 3));     
}

Icosahedron::Icosahedron(Vec3f center, float radius) {
    const float ang = static_cast<float>(1 / std::sqrt(5));

    this->center = center;

    float x = radius * std::cos(std::asin(ang));
    float y = radius * ang;

    Vec3f startPoint(x, y, 0);

    vertices.push_back(center + startPoint);
    vertices.push_back(center + startPoint.cycle());
    vertices.push_back(center + startPoint.cycle().cycle());
    vertices.push_back(center + startPoint.negate(false, true, false));
    vertices.push_back(center + startPoint.cycle().negate(false, true, false));
    vertices.push_back(center + startPoint.cycle().negate(false, true, true));
    vertices.push_back(center + startPoint.cycle().cycle().negate(true, false, false));
    vertices.push_back(center + startPoint.cycle().negate(false, false, true));
    vertices.push_back(center + startPoint.negate(true, false, false));
    vertices.push_back(center + startPoint.negate(true, true, false));
    vertices.push_back(center + startPoint.cycle().cycle().negate(false, false, true));
    vertices.push_back(center + startPoint.cycle().cycle().negate(true, false, true));

    tris.push_back(Triangle(0, 1, 2));
    tris.push_back(Triangle(0, 3, 2));
    tris.push_back(Triangle(3, 4, 2));
    tris.push_back(Triangle(3, 5, 4));
    tris.push_back(Triangle(4, 6, 2));
    tris.push_back(Triangle(6, 1, 2));
    tris.push_back(Triangle(0, 1, 7));
    tris.push_back(Triangle(1, 8, 6));
    tris.push_back(Triangle(6, 8, 9));
    tris.push_back(Triangle(9, 4, 6));
    tris.push_back(Triangle(4, 9, 5));
    tris.push_back(Triangle(7, 8, 1));
    tris.push_back(Triangle(10, 0, 3));
    tris.push_back(Triangle(10, 5, 3));
    tris.push_back(Triangle(10, 7, 0));
    tris.push_back(Triangle(11, 5, 10));
    tris.push_back(Triangle(11, 5, 9));
    tris.push_back(Triangle(11, 9, 8));
    tris.push_back(Triangle(11, 8, 7));
    tris.push_back(Triangle(11, 7, 10));
}

Sphere::Sphere(Vec3f center, float radius) {
    Icosahedron start(center, radius);

    this->center = center;
    this->radius = radius;

    tris.assign(start.tris.begin(), start.tris.end());
    vertices.assign(start.vertices.begin(), start.vertices.end());

    std::vector<Triangle> newTris;
    for (int i = 0; i < 2; i++) {
        for (Triangle &tri : tris) {
            int m01 = getMidpoint(tri.a, tri.b);
            int m12 = getMidpoint(tri.b, tri.c);
            int m20 = getMidpoint(tri.c, tri.a);

            newTris.push_back(Triangle(tri.a, m01, m20));
            newTris.push_back(Triangle(tri.b, m12, m01));
            newTris.push_back(Triangle(tri.c, m20, m12));
            newTris.push_back(Triangle(m01, m12, m20));
        }
        tris.swap(newTris);
    }

}

int Sphere::getMidpoint(int i1, int i2) {

    std::pair<int, int> key = std::minmax(i1, i2);
    auto it = midPointIndex.find(key);
    if (it != midPointIndex.end())
        return it->second;

    Vec3f v1 = vertices[i1], v2 = vertices[i2];
    Vec3f mid = (v1 + v2) * 0.5f;
    mid = mid.normalize() * radius;
    int newIndex = static_cast<int>(vertices.size());
    vertices.push_back(mid);
    midPointIndex[key] = newIndex;
    return newIndex;
}