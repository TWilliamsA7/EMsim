#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <vector>

#include <SDL.h>
#include "Vec3.h"

// NXN refers to both unmapped and describes the number of mapped keys
enum class Key {
    Q, W, E, A, S, D, NXN
};

struct ProjPos {
    int x;
    int y;

    ProjPos (int x, int y) : x(x), y(y) {}
};

class Camera {
    public:
        Vec3f target;
        float distance;
        float yaw;
        float pitch;
        float rotateSpeed;
        float panSpeed;

        float focalLength;
        float near;

        // Distance: Zoom In/Out
        // Yaw: Rotate around Y-axis
        // Pitch: Tilt Up and Down
        // Target: Point of Orbit

        // Camera vectors
        Vec3f camPos;
        Vec3f forward;
        Vec3f right;
        Vec3f up;

        Camera();

        Vec3f position() const;

        // Recompute forward, right, up vectors of camera
        // Call before projections or panning
        void computeVectors();
};

class Renderer3D {
    public:

        // Class Constructors
        Renderer3D(int width, int height);
        ~Renderer3D();

        void run();
        
    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        int width, height;
        float scale;
        
        Camera cam;

        std::vector<bool> key_map;

        // Key Input Handling
        void handleInput(SDL_Event *e);
        Key mapKey(SDL_Keycode key);

        // Camera Functions
        void rotateCam(SDL_Event* e, int* lastX, int* lastY);
        void panCam(SDL_Event* e, int* lastX, int* lastY);
        bool clipAgainstCam(Vec3f& A, Vec3f& B);

        // Projection Functions
        Vec3f screenProj(const Vec3f& point);
        ProjPos camToWorld(const Vec3f& C);

        // Draw functions
        void drawPoint(const Vec3f& point);
        void drawLine(const Vec3f& point1, const Vec3f& point2);
        
        // Render current frame
        void renderFrame();

        // Draw figures
        void drawIcosahedron(const Vec3f& center, float radius);

};



#endif // __RENDERER_H__