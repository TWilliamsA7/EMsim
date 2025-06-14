#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <array>
#include <vector>

#include <SDL.h>
#include "Vec3.h"
#include "Objects.h"

// NXN refers to both unmapped and describes the number of mapped keys
enum class Key {
    Q, W, E, A, S, D, NXN
};

struct ProjPos {
    int x;
    int y;

    ProjPos () : x(0), y(0) {}
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
        
        
        // Holds all objects of the scene
        std::vector<Object*> scene;
        Camera cam;
        std::vector<bool> key_map;

        Vec3f lightDir;

        // Initialization
        void loadScene();

        // Render current frame
        void renderFrame();

        // Depth Consideration Functions
        // These functions sort scene in display order based on center of objects (not perfect but it's enough)
        void sortScene(int L, int R);
        void sortSceneMerge(int L, int R, int mid);
        bool isInFront(const Object* A, const Object* B);

        // Projection Functions
        // World Space => Camera Space => Screen Space
        Vec3f worldToCam(const Vec3f& point);
        ProjPos camToScreen(const Vec3f& C);

        // Draw functions
        void drawPoint(const Vec3f& point);
        void drawLine(const Vec3f& point1, const Vec3f& point2);
        void drawObject(const Object* obj);

        // Fill functions
        void geometryObjectFill(const Object* obj);
        // Helper fill functions
        std::vector<std::array<Vec3f, 3>> clipAgainstNearPlane(const Vec3f& A, const Vec3f& B, const Vec3f& C);
        bool allVertsOutside(std::array<ProjPos, 3> ndc);

        // Color helper function
        void setDrawColor(SDL_Color color);

        
        // Camera Functions
        void rotateCam(SDL_Event* e, int* lastX, int* lastY);
        void panCam(SDL_Event* e, int* lastX, int* lastY);
        bool clipLineAgainstCam(Vec3f& A, Vec3f& B);
        
        // Key Input Handling
        void handleInput(SDL_Event *e);
        Key mapKey(SDL_Keycode key);
};



#endif // __RENDERER_H__