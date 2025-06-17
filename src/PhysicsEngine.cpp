#include "PhysicsEngine.h"
#include "Matrix3.h"

PhysicsObject::PhysicsObject(Object* o) {
    obj = o;
}

void PhysicsObject::updateRotation(Vec3f rot) {

    rotation = rotation + rot;
    if (rotation.x > 2 * PI) rotation.x -= 2 * PI;
    if (rotation.y > 2 * PI) rotation.y -= 2 * PI;
    if (rotation.z > 2 * PI) rotation.z -= 2 * PI;

    float c, s;
    c = std::cos(rot.x);
    s = std::sin(rot.x);
    std::array<float, 9> xVal = {1, 0, 0, 0, c, -s, 0, s, c};
    Matrix3 rotMatX = Matrix3(xVal);
    c = std::cos(rot.y);
    s = std::sin(rot.y);
    std::array<float, 9> yVal = {c , 0, s, 0, 1, 0, -s, 0, c};
    Matrix3 rotMatY = Matrix3(yVal);
    c = std::cos(rot.z);
    s = std::sin(rot.z);
    std::array<float, 9> zVal = {c, -s, 0, s, c, 0, 0, 0, 1};
    Matrix3 rotMatZ = Matrix3(zVal);

    Matrix3 rotMat = rotMatY * rotMatX * rotMatZ;

    for (Vec3f& vert : obj->vertices) {
        vert = vert - obj->center;
        vert = rotMat * vert;
        vert = vert + obj->center;
    }

}



void PhysicsObject::Translate() {

    // Update Velocity based on acceleration
    velocity = velocity + acceleration * (1 / 60.f);

    // To account for 60 frames per second
    Vec3f transVector = velocity * (1 / 60.f);

    obj->center = obj->center + transVector;
    for (Vec3f& vert : obj->vertices)
        vert = vert + transVector;

}

void PhysicsObject::Rotate() {
    angularVelocity = angularVelocity + angularAcceleration * (1 / 60.f);
    Vec3f rotVector = angularVelocity * (1 /60.f);
    updateRotation(rotVector);
}