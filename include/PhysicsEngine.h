#ifndef __PHYSICSENGINE_H__
#define __PHYSICSENGINE_H__

#include "Objects.h"

class PhysicsObject {
    public:
        Object* obj;

        float mass;
        float charge;

        Vec3f velocity;
        Vec3f acceleration;
        Vec3f angularVelocity;
        Vec3f angularAcceleration;
        Vec3f rotation;

        // Force being felt by the object
        Vec3f force;

        PhysicsObject(Object* o);

        void updateRotation(Vec3f rot);
        void Rotate();
        void Translate();
};

#endif // __PHYSICSENGINE_H__