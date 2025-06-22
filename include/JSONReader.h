#ifndef __JSONREADER_H__
#define __JSONREADER_H__

#include <vector>
#include <string>
#include "Vec3.h"

class ObjectModel {

    public:
        enum Shape {
            SPHERE,
            TETRAHEDRON,
            ICOSAHEDRON,
        };

        struct Color {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;

            std::string toString() {return "(" + std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b) + " " + std::to_string(a) + ")";};
        };

        // Object name
        std::string name;
        Shape shape;

        Vec3f center;
        float radius;
        Color color;
        bool wireframe;
        float mass;
        float charge;

        Vec3f initVelocity;
        Vec3f initAccel;
        Vec3f initAngVelocity;
        Vec3f initAngAccel;
};

class Simulation {

    
    std::string retrieveString(std::string line, int pos);
    float retrieveFloat(std::string line, int pos);
    Vec3f retrieveVector(std::string line);
    ObjectModel::Color retrieveColor(std::string line);
    bool retrieveBool(std::string line);
    
    void setWindowParam(std::string line, std::string memberName, int pos);
    void setCameraParam(std::string line, std::string memberName, int pos);
    void setObjectParam(ObjectModel& objM, std::string line, std::string memberName, int pos);
    
    public:
        Simulation(char* filename);
        void displaySim();
        bool loadedSuccess;
        
        // Name of the simulation
        std::string name;
        
        // Window Members
        int width;
        int height;
        float scale;
        Vec3f lightDir;
        
        // Camera Members
        float distance;
        float rotateSpeed;
        float panSpeed;
        float zoomSpeed;
        float focalLength;
        float near;
        
        // Objects
        std::vector<ObjectModel> objModels;
};

#endif // __JSONREADER_H__