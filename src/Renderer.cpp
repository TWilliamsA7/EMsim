#include "Renderer.h"
#include <iostream>
#include <algorithm>

// Camera Functions
Camera::Camera() {
    this->target = Vec3f();

    this->distance = 5.0f;
    this->yaw = 0.0f;
    this->pitch = 0.0f;
    this->rotateSpeed = 0.005f;
    this->panSpeed = 0.01f;
    this->focalLength = 2.0f;
    this->near = 0.1f;

    this->computeVectors();
}

Vec3f Camera::position() const {
    float x = target.x + distance * std::cos(pitch) * std::sin(yaw);
    float y = target.y + distance * std::sin(pitch);
    float z = target.z + distance * std::cos(pitch) * std::cos(yaw);
    return Vec3f(x, y, z);
}

// Recompute forward, right, up vectors of camera
// Call before projections or panning
void Camera::computeVectors() {
    camPos = position();
    forward = (target - camPos).normalize();
    Vec3f v(0, 1, 0);
    right  = forward.cross(v).normalize();
    up = right.cross(forward).normalize();
}



Renderer3D::Renderer3D(int width, int height) : width(width), height(height) {
    SDL_Init(SDL_INIT_VIDEO);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Viewport",
                            SDL_WINDOWPOS_CENTERED, 
                            SDL_WINDOWPOS_CENTERED, 
                            width, height, 
                            SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }



    cam = Camera();
    scale = 100.0f;
    
    for (int i = 0; i < (int) Key::NXN; i++) key_map.push_back(false);
}

Renderer3D::~Renderer3D() {

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    for (Object* obj : scene) 
        delete obj;

}


// Load all objects of the scene into the Renderer
void Renderer3D::loadScene() {
    // For now define objects in this function will add more later on

    scene.push_back(new Sphere(Vec3f(), 2));
    scene.push_back(new Sphere(Vec3f(2, 1, 4), 1));
}

// Draws a point on the window
void Renderer3D::drawPoint(const Vec3f &point) {
    // ProjPos projPoint = screenProj(point);

    Vec3f C = screenProj(point);
    ProjPos p = camToWorld(C);

    // Only Draw if within the screen
    if (p.x > width || p.x < 0 || p.y > height || p.y < 0) {
        return;
    }

    SDL_RenderDrawPoint(renderer, p.x, p.y);
}

// Draws a line from point1 to point2
void Renderer3D::drawLine(const Vec3f& point1, const Vec3f& point2){

    Vec3f C1 = screenProj(point1);
    Vec3f C2 = screenProj(point2);

    // If the line is completely behind the camera
    if (!clipAgainstCam(C1, C2)) return;
    
    ProjPos p1 = camToWorld(C1);
    ProjPos p2 = camToWorld(C2);

    if ((p1.x < 0 && p2.x < 0) || (p1.x > width && p2.x > width ) ||
        (p1.y < 0 && p2.y < 0) || (p1.y > height && p2.y > height)) {
            return;
        }

    SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);

}

// Draws an object
void Renderer3D::drawObject(const Object* obj) {
    for (Triangle tri : obj->tris) {
        drawLine(obj->vertices[tri.a], obj->vertices[tri.b]);
        drawLine(obj->vertices[tri.b], obj->vertices[tri.c]);
        drawLine(obj->vertices[tri.c], obj->vertices[tri.a]);
    }
}

// Convert from cam coords to screen coords
ProjPos Renderer3D::camToWorld(const Vec3f& C) {
    float x_proj = (cam.focalLength / C.z) * C.x;
    float y_proj = (cam.focalLength / C.z) * C.y;
    int x = static_cast<int>(x_proj * scale + width * 0.5f);
    int y = static_cast<int>(-y_proj * scale + height * 0.5f);
    return ProjPos(x, y);
}

bool Renderer3D::clipAgainstCam(Vec3f& A, Vec3f& B) {
    // If the line is completely behind the camera
    if (A.z < cam.near && B.z < cam.near) return false;

    // Reduce the clipped vector to the point where it is visible
    if (A.z < cam.near) {
        float t = (cam.near - A.z) / (B.z - A.z);
        A = A + (B - A) * t;
    } else if (B.z < cam.near) {
        float t = (cam.near - B.z) / (A.z - B.z);
        B = B + (A - B) * t;
    }
    return true;
}

Vec3f Renderer3D::screenProj(const Vec3f& point) {
    // Transform into camera space
    Vec3f toPoint = point - cam.camPos;
    float camX = toPoint.dot(cam.right);
    float camY = toPoint.dot(cam.up);
    float camZ = toPoint.dot(cam.forward);  

    return Vec3f(camX, camY, camZ);   
}

void Renderer3D::renderFrame() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Clear to black
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White points

    // Draw a point representing the origin
    drawPoint(Vec3f());

    for (Object* obj : scene) {
        drawObject(obj);
    }

    SDL_RenderPresent(renderer);
}

void Renderer3D::run() {
    bool quit = false;

    // Camera control
    bool rotating = false;
    bool panning = false;
    int lastX=0, lastY=0;

    SDL_Event e;
    loadScene();

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            switch (e.type)
            {
                // On window closure
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_MOUSEMOTION:
                    if (rotating) {
                        rotateCam(&e, &lastX, &lastY);
                    } else if (panning) {
                        panCam(&e, &lastX, &lastY);
                    }
                    break;

                case SDL_MOUSEWHEEL:
                    cam.distance *= (e.wheel.y > 0 ? 0.9f : 1.1f);
                    cam.distance = std::max(cam.distance, 0.1f);
                    break;

                // On Mouse Down
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_RIGHT) {
                        rotating = true;
                        lastX = e.button.x;
                        lastY = e.button.y;
                    } else if (e.button.button == SDL_BUTTON_LEFT &&
                                (SDL_GetModState() & KMOD_CTRL)) {
                        panning = true;
                        lastX = e.button.x;
                        lastY = e.button.y;
                    }
                    break;

                // On Mouse Up
                case SDL_MOUSEBUTTONUP:
                    if (e.button.button == SDL_BUTTON_RIGHT)
                        rotating = false;
                    else if (e.button.button == SDL_BUTTON_LEFT)
                        panning = false;
                    break;
            }

        }

        cam.computeVectors();
        renderFrame();
        SDL_Delay(16);  // ~60 FPS
    }
}

void Renderer3D::rotateCam(SDL_Event* e, int* lastX, int* lastY) {
    int dx = e->motion.x - *lastX;
    int dy = e->motion.y - *lastY;
    
    cam.yaw += dx * cam.rotateSpeed;
    cam.pitch += dy * cam.rotateSpeed;
    
    // Restrict pitch
    cam.pitch = (cam.pitch > 1.5f) ? 1.5f : cam.pitch;
    cam.pitch = (cam.pitch < -1.5f) ? -1.5f : cam.pitch;
    
    *lastX = e->motion.x;
    *lastY = e->motion.y;
}

void Renderer3D::panCam(SDL_Event* e, int* lastX, int* lastY) {
    int dx = e->motion.x - *lastX;
    int dy = e->motion.y - *lastY;
    
    // Determine vectors of camera orientation
    cam.computeVectors();

    float panFactor = cam.distance / cam.focalLength * cam.panSpeed;
    Vec3f delta_world = cam.right * (-dx * panFactor) + cam.up * (dy * panFactor);
    
    cam.target = cam.target + delta_world;
    *lastX = e->motion.x;
    *lastY = e->motion.y;
}

void Renderer3D::drawIcosahedron(const Vec3f& center, float radius) {
    // ang is the value of sine of the angle formed from the center to any given point of the figure
    const float ang = static_cast<float>(1 / std::sqrt(5));

    // Compute starting point
    float x = radius * std::cos(std::asin(ang));
    float y = radius * ang;

    Vec3f startPoint(x, y, 0);

    // Stores points in order of drawing (Contains Duplicate Points but Not Duplicate Lines)
    std::vector<Vec3f> points;

    // Add points in order of traversal
    points.push_back(center + startPoint);
    points.push_back(center + startPoint.cycle());
    points.push_back(center + startPoint.cycle().cycle());
    points.push_back(center + startPoint);
    points.push_back(center + startPoint.negate(false, true, false));
    points.push_back(center + startPoint.cycle().cycle());
    points.push_back(center + startPoint.cycle().negate(false, true, false));
    points.push_back(center + startPoint.negate(false, true, false));
    points.push_back(center + startPoint.cycle().negate(false, true, true));
    points.push_back(center + startPoint.cycle().negate(false, true, false));
    points.push_back(center + startPoint.cycle().cycle().negate(true, false, false));
    points.push_back(center + startPoint.cycle().cycle());
    points.push_back(center + startPoint.cycle().cycle().negate(true, false, false));
    points.push_back(center + startPoint.cycle());
    points.push_back(center + startPoint.cycle().negate(false, false, true));
    points.push_back(center + startPoint);
    points.push_back(center + startPoint.cycle().cycle().negate(false, false, true));
    points.push_back(center + startPoint.cycle().negate(false, false, true));
    points.push_back(center + startPoint.negate(true, false, false));
    points.push_back(center + startPoint.cycle());
    points.push_back(center + startPoint.cycle().cycle().negate(true, false, false));
    points.push_back(center + startPoint.negate(true, false, false));
    points.push_back(center + startPoint.negate(true, true, false));
    points.push_back(center + startPoint.cycle().cycle().negate(true, false, false));
    points.push_back(center + startPoint.negate(true, true, false));
    points.push_back(center + startPoint.cycle().negate(false, true, false));
    points.push_back(center + startPoint.negate(true, true, false));
    points.push_back(center + startPoint.cycle().negate(false, true, true));
    points.push_back(center + startPoint.cycle().cycle().negate(false, false, true));
    points.push_back(center + startPoint.negate(false, true, false));
    points.push_back(center + startPoint.cycle().cycle().negate(false, false, true));
    points.push_back(center + startPoint.cycle().cycle().negate(true, false, true));
    points.push_back(center + startPoint.cycle().negate(false, true, true));
    points.push_back(center + startPoint.cycle().cycle().negate(true, false, true));
    points.push_back(center + startPoint.cycle().negate(false, false, true));
    points.push_back(center + startPoint.cycle().cycle().negate(true, false, true));
    points.push_back(center + startPoint.negate(true, false, false));
    points.push_back(center + startPoint.cycle().cycle().negate(true, false, true));
    points.push_back(center + startPoint.negate(true, true, false));

    for (int i = 0; i < (points.size() - 1); i++) {
        drawLine(points[i], points[i + 1]);
    }
}

void Renderer3D::handleInput(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP) {
        Key key = this->mapKey(e->key.keysym.sym);
        if (key != Key::NXN) {
            if (e->type == SDL_KEYDOWN) {
                key_map[(int) key] = true;
            } else if (e->type == SDL_KEYUP) {
                key_map[(int) key] = false;
            }   
        }
    }
}

Key Renderer3D::mapKey(SDL_Keycode key) {
    switch (key)
    {
        case SDLK_q : return Key::Q;
        case SDLK_w : return Key::W;
        case SDLK_e : return Key::E;
        case SDLK_a : return Key::A;
        case SDLK_s : return Key::S;
        case SDLK_d : return Key::D;
        default: return Key::NXN;
    }
}