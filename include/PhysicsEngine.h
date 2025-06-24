#ifndef __PHYSICSENGINE_H__
#define __PHYSICSENGINE_H__

#include "Objects.h"

const float EpsilonNaught = 8.854e-12;
const float ColoumbConstant = 1 / (4 * PI * EpsilonNaught);
const float GravitationalConstant = 6.6743e-11;

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
    
    
    PhysicsObject(Object* o, float mass, float charge);
    
    void Rotate(float dt);

    private: 
        void updateRotation(Vec3f rot);
    
};

class Field {
    public:
        enum Type {
            Electric,
            Magnetic
        };

        Type type;
        Vec3f direction;
        float strength;
        Field(Type t, float str, Vec3f dir) : type(t), direction(dir.normalize()), strength(str) {};
};

class PhysicsEngine {
    public:
        void integrateForward(const std::vector<PhysicsObject*> scene, const std::vector<Field> fields, float t, float dt);
        void eulerRotate(const std::vector<PhysicsObject*> scene, float dt);

    private:

        struct State {
            Vec3f r;
            Vec3f v;
        };

        State getState(const PhysicsObject* p);
        State stateAdd(const State& s, const State& d, float dt);
        State evaluate(const std::vector<PhysicsObject*> scene, const std::vector<Field> fields, int i, const State& s, float t);

        Vec3f computeForce(const std::vector<PhysicsObject*> scene, const std::vector<Field> fields, int i);
        Vec3f computeColoumbForce(const PhysicsObject* target, const PhysicsObject* emitter);
        Vec3f computeGravitationalForce(const PhysicsObject* target, const PhysicsObject* emitter);
        Vec3f computeElectricFieldForce(const PhysicsObject* target, const Field E);
        Vec3f computeMagneticFieldForce(const PhysicsObject* target, const Field B);
};

// State getState(PhysicsObject* p);
// State stateAdd(const State& s, const Deriv& d, float dt);
//  Deriv evaluate(int i, const std::vector<PhysicsObject*>& objects, const State& s, float t);
// void integrateRK4(const std::vector<PhysicsObject*>& objects, float t, float dt);


#endif // __PHYSICSENGINE_H__