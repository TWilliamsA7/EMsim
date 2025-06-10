#include "Objects.h"

#include <iostream>

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