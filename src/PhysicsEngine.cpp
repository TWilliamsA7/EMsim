#include "PhysicsEngine.h"
#include "Matrix3.h"
#include <iostream>

PhysicsObject::PhysicsObject(Object* o, float mass, float charge) {
    obj = o;
    this->mass = mass;
    this->charge = charge;
}

void PhysicsObject::Rotate(float dt) {
    angularVelocity = angularVelocity + angularAcceleration * dt;
    Vec3f rotVector = angularVelocity * dt;
    updateRotation(rotVector);
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

void PhysicsEngine::eulerRotate(const std::vector<PhysicsObject*> scene, float dt) {
    for (auto Pobj : scene)
        Pobj->Rotate(dt);
}

void PhysicsEngine::integrateForward(const std::vector<PhysicsObject*> scene, const std::vector<Field> fields, float t, float dt) {
    int N = scene.size();
    // For each particle, we’ll compute k1..k4
    std::vector<State>  y0(N), k1(N), k2(N), k3(N), k4(N);
    std::vector<State>  tempY(N);

    // 1) Collect initial states
    for (int i = 0; i < N; ++i) {
        y0[i] = getState(scene[i]);
    }

    // 2) k1 = f(t, y0)
    for (int i = 0; i < N; ++i) {
        auto d = evaluate(scene, fields, i, y0[i], t);
        k1[i] = { d.r, d.v };
    }

    // 3) k2 = f(t + dt/2, y0 + k1/2)
    for (int i = 0; i < N; ++i) {
        tempY[i] = stateAdd(y0[i], k1[i], 0.5f);
    }
    for (int i = 0; i < N; ++i) {
        auto d = evaluate(scene, fields, i, tempY[i], t + dt*0.5f);
        k2[i] = { d.r, d.v };
    }

    // 4) k3 = f(t + dt/2, y0 + k2/2)
    for (int i = 0; i < N; ++i) {
        tempY[i] = stateAdd(y0[i], k2[i], 0.5f);
    }
    for (int i = 0; i < N; ++i) {
        auto d = evaluate(scene, fields, i, tempY[i], t + dt*0.5f);
        k3[i] = { d.r, d.v };
    }

    // 5) k4 = f(t + dt, y0 + k3)
    for (int i = 0; i < N; ++i) {
        tempY[i] = stateAdd(y0[i], k3[i], 1.0f);
    }
    for (int i = 0; i < N; ++i) {
        auto d = evaluate(scene, fields, i, tempY[i], t + dt);
        k4[i] = { d.r, d.v };
    }

    // 6) Combine into final state
    for (int i = 0; i < N; ++i) {
        Vec3f dr = (k1[i].r + k2[i].r*2.0f + k3[i].r*2.0f + k4[i].r) * (1.f / 6.0f);
        Vec3f dv = (k1[i].v + k2[i].v*2.0f + k3[i].v*2.0f + k4[i].v) * (1.f / 6.0f);

        scene[i]->obj->center = scene[i]->obj->center + (dr * dt);
        for (Vec3f& vert : scene[i]->obj->vertices)
            vert = vert + (dr * dt);

        scene[i]->velocity = scene[i]->velocity + (dv * dt);   // advance velocity
    }
}

PhysicsEngine::State PhysicsEngine::getState(const PhysicsObject* p) {
    return { p->obj->center, p->velocity };
}

PhysicsEngine::State PhysicsEngine::stateAdd(const State& s, const State& d, float dt) {
    return { s.r + (d.r * dt), s.v + (d.v * dt) };
}

PhysicsEngine::State PhysicsEngine::evaluate(const std::vector<PhysicsObject*> scene, const std::vector<Field> fields, int i, const State& s, float t) {
    Vec3f F = computeForce(scene, fields, i);
    return { s.v, F * (1.f / scene[i]->mass) };
}

Vec3f PhysicsEngine::computeForce(const std::vector<PhysicsObject*> scene, const std::vector<Field> fields, int i) {
    int N = scene.size();

    Vec3f force = Vec3f();
    for (int j = 0; j < N; j++) {
        if (i == j) continue;
        force = force + computeColoumbForce(scene[i], scene[j]);
        force = force + computeGravitationalForce(scene[i], scene[j]);
    }

    N = fields.size();
    for (int j = 0; j < N; j++) {
        Field f = fields.at(j);
        switch (f.type)
        {
            case Field::Type::Electric:
                force = force + computeElectricFieldForce(scene[i], f);
                break;
            case Field::Type::Magnetic:
                force = force + computeMagneticFieldForce(scene[i], f);
                break;
        }
    }

    return force;
}

Vec3f PhysicsEngine::computeColoumbForce(const PhysicsObject* target, const PhysicsObject* emitter) {
    float scale = ColoumbConstant * target->charge * emitter->charge;
    Vec3f r = target->obj->center - emitter->obj->center;
    float dist = r.magnitude();
    scale /= dist * dist * dist;
    return r * scale;
}

Vec3f PhysicsEngine::computeGravitationalForce(const PhysicsObject* target, const PhysicsObject* emitter) {
    float scale = -GravitationalConstant * target->mass * emitter->mass;
    Vec3f r = target->obj->center - emitter->obj->center;
    float dist = r.magnitude();
    scale /= dist * dist * dist;
    return r * scale;
}

Vec3f PhysicsEngine::computeElectricFieldForce(const PhysicsObject* target, const Field E) {
    float mag = E.strength * target->charge;
    return E.direction * mag;
}

Vec3f PhysicsEngine::computeMagneticFieldForce(const PhysicsObject* target, const Field B) {
    Vec3f qv = target->velocity * target->charge;
    Vec3f Bvect = B.direction * B.strength;
    return qv.cross(Bvect);
}