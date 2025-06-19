#ifndef __PHYSICSENGINE_H__
#define __PHYSICSENGINE_H__

#include "Objects.h"

const float EpsilonNaught = 8.854e-12;
const float ColoumbConstant = 1 / (4 * PI * EpsilonNaught);


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
    
    PhysicsObject(Object* o, float mass, float charge);
    
    void Rotate();
    void updateRotation(Vec3f rot);
    void Translate();
    
    
    // Used for integrating for positon and velocity
};

class PhysicsEngine {
    public:
        void integrateForward(const std::vector<PhysicsObject*> scene, float t, float dt);

    private:

        struct State {
            Vec3f r;
            Vec3f v;
        };

        State getState(const PhysicsObject* p);
        State stateAdd(const State& s, const State& d, float dt);
        State evaluate(const std::vector<PhysicsObject*> scene, int i, const State& s, float t);

        Vec3f computeForce(const std::vector<PhysicsObject*> scene, int i);
        Vec3f computeColoumbForce(const PhysicsObject* target, const PhysicsObject* emitter);
};

// State getState(PhysicsObject* p);
// State stateAdd(const State& s, const Deriv& d, float dt);
//  Deriv evaluate(int i, const std::vector<PhysicsObject*>& objects, const State& s, float t);
// void integrateRK4(const std::vector<PhysicsObject*>& objects, float t, float dt);


#endif // __PHYSICSENGINE_H__