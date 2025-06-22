#include "Renderer.h"
#include <iostream>
#include <algorithm>

/*******************************************************************
                        CAMERA FUNCTIONS
********************************************************************/

// Initialize Camera Settings
Camera::Camera(Simulation* sim) {
    this->target = Vec3f();

    // this->distance = sim->distance;
    // this->yaw = 0.0f;
    // this->pitch = 0.0f;
    // this->rotateSpeed = sim->rotateSpeed;
    // this->panSpeed = sim->panSpeed;
    // this->zoomSpeed = sim->zoomSpeed;
    // this->focalLength = sim->focalLength;
    // this->near = sim->near;
    this->distance = 5.0f;
    this->yaw = 0.0f;
    this->pitch = 0.0f;
    this->rotateSpeed = 0.005f;
    this->panSpeed = 0.01f;
    this->zoomSpeed = 0.1f;
    this->focalLength = 4.0;
    this->near = 0.1;

    this->computeVectors();
}

Camera::Camera() {
    this->target = Vec3f();
    this->distance = 5.0f;
    this->yaw = 0.0f;
    this->pitch = 0.0f;
    this->rotateSpeed = 0.005f;
    this->panSpeed = 0.01f;
    this->zoomSpeed = 0.1f;
    this->focalLength = 4.0;
    this->near = 0.1;

    this->computeVectors();
}

// Return the position of the camera
Vec3f Camera::position() const {
    float x = target.x + distance * std::cos(pitch) * std::sin(yaw);
    float y = target.y + distance * std::sin(pitch);
    float z = target.z + distance * std::cos(pitch) * std::cos(yaw);
    return Vec3f(x, y, z);
}

// Update forward, right, up vectors of camera
void Camera::computeVectors() {
    camPos = position();
    forward = (target - camPos).normalize();
    Vec3f v(0, 1, 0);
    right  = forward.cross(v).normalize();
    up = right.cross(forward).normalize();
}


/*******************************************************************
                        RENDERER FUNCTIONS
********************************************************************/

// Initialize renderer window with dimensions width x height
Renderer3D::Renderer3D(Simulation* sim) {
    // Initialize Video
    SDL_Init(SDL_INIT_VIDEO);

    // Validate correct opening
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Create window
    window = SDL_CreateWindow(sim->name.c_str(),
                            SDL_WINDOWPOS_CENTERED, 
                            SDL_WINDOWPOS_CENTERED, 
                            sim->width, sim->height, 
                            SDL_WINDOW_SHOWN);

    this->width = sim->width;
    this->height = sim->height;

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    // Create Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    this->cam = Camera();
    this->lightDir = sim->lightDir.normalize();
    this->scale = sim->scale;
    this->sim = sim;
}

// Called at program termination to clean up
Renderer3D::~Renderer3D() {

    // Destroy SDL Instance
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // Deallocate dynamically allocated scene objects
    for (PhysicsObject* Pobj : scene) {
        delete Pobj->obj;
        delete Pobj;
    }

}


// EXECUTION AND INITIALIZATION


// Public function with main execution loop
void Renderer3D::run() {
    bool quit = false;

    // Camera control
    bool rotating = false;
    bool panning = false;
    int lastX=0, lastY=0;
    float simTime = 0.0f;
    float dt = 1 / 60.0f;

    SDL_Event e;

    // Load initial scene
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
                    // Zoom In/Out
                    cam.distance *= (e.wheel.y > 0 ? 1.f - cam.zoomSpeed : 1.f + cam.zoomSpeed);
                    cam.distance = std::max(cam.distance, 0.1f);
                    break;

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

                case SDL_MOUSEBUTTONUP:
                    if (e.button.button == SDL_BUTTON_RIGHT)
                        rotating = false;
                    else if (e.button.button == SDL_BUTTON_LEFT)
                        panning = false;
                    break;
            }
        }

        // Recompute camera vectors before each frame
        cam.computeVectors();

        phyeng.integrateForward(scene, simTime, dt);
        phyeng.eulerRotate(scene, dt);
        simTime += dt;
        renderFrame();
        SDL_Delay(16);  // ~60 FPS
    }
}

// Load objects of the scene into the Renderer
void Renderer3D::loadScene() {
    PhysicsObject* hold = nullptr;
    for (auto objM : sim->objModels) {
        hold = loadObjectModel(objM);
        scene.push_back(hold);
    }
}

PhysicsObject* Renderer3D::loadObjectModel(ObjectModel objM) {
    Object* obj = nullptr;

    SDL_Color color = {objM.color.r, objM.color.g, objM.color.b, objM.color.a};

    switch (objM.shape)
    {
        case ObjectModel::Shape::SPHERE:
            obj = new Sphere(objM.center, objM.radius, color, objM.wireframe);
            break;
        case ObjectModel::Shape::ICOSAHEDRON:
            obj = new Icosahedron(objM.center, objM.radius, color, objM.wireframe);
            break;
        case ObjectModel::Shape::TETRAHEDRON:
            obj = new Tetrahedron(objM.center, objM.radius, color, objM.wireframe);
            break;
        default: // default to a sphere
            obj = new Sphere(objM.center, objM.radius, color, objM.wireframe);
    }

    PhysicsObject* hold = new PhysicsObject(obj, objM.mass, objM.charge);
    hold->velocity = objM.initVelocity;
    hold->acceleration = objM.initAccel;
    hold->angularVelocity = objM.initAngVelocity;
    hold->angularAcceleration = objM.initAngAccel;

    return hold;
}

// RENDER FUNCTIONS


// Clears the screen the draws the objects of the scene
void Renderer3D::renderFrame() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Clear to black
    SDL_RenderClear(renderer);
    
    //Sort scene array by depth against camera
    sortScene(0, static_cast<int>(scene.size() - 1));
    
    for (PhysicsObject* Pobj : scene) {
        drawObject(Pobj->obj);
    }
    
    // Draw a point representing the origin
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White points
    drawCrosshair();

    SDL_RenderPresent(renderer);
}

// Sorts objects in a scene from furthest to closest (Potentially replace with quick sort in the future)
void Renderer3D::sortScene(int L, int R) {
    if (L < R) {
        int mid = (L + R) / 2;
        sortScene(L, mid);
        sortScene(mid + 1, R);
        sortSceneMerge(L, R, mid);
    }
}

// Helper functon for sortScene 
void Renderer3D::sortSceneMerge(int L, int R, int mid) {

    int sizeLeft = mid - L + 1;
    int sizeRight = R - mid;

    PhysicsObject** left = (PhysicsObject**) malloc(sizeof(PhysicsObject*) * sizeLeft);
    PhysicsObject** right = (PhysicsObject**) malloc(sizeof(PhysicsObject*) * sizeRight);

    for (int i = 0; i < sizeLeft; i++)
        left[i] = scene.at(L + i);
    
    for (int i = 0; i < sizeRight; i++)
        right[i] = scene.at(mid + i + 1);

    int i = 0, j = 0, k = L;

    while (i < sizeLeft && j < sizeRight) {
        if (isInFront(left[i]->obj, right[j]->obj))
            scene.at(k++) = right[j++];
        else
            scene.at(k++) = left[i++];
    }

    while (i < sizeLeft)
        scene.at(k++) = left[i++];
    while (j < sizeRight)
        scene.at(k++) = right[j++];
    
    free(left);
    free(right);

}

// Returns true if A should be displayed in front of B
bool Renderer3D::isInFront(const Object* A, const Object* B) {
    Vec3f Acenter = worldToCam(A->center);
    Vec3f Bcenter = worldToCam(B->center);

    return (Acenter.z < Bcenter.z) ? true : false;
}

// DRAW FUNCTIONS

// Draws a point on the window
void Renderer3D::drawPoint(const Vec3f &point) {
    // Make transformations
    Vec3f C = worldToCam(point);
    ProjPos p = camToScreen(C);

    // Only Draw if within the screen
    if (p.x > width || p.x < 0 || p.y > height || p.y < 0) {
        return;
    }

    SDL_RenderDrawPoint(renderer, p.x, p.y);
}

// Draws a line from point1 to point2
void Renderer3D::drawLine(const Vec3f& point1, const Vec3f& point2){

    // Transform relative to the camera
    Vec3f C1 = worldToCam(point1);
    Vec3f C2 = worldToCam(point2);

    // If the line is completely behind the camera
    if (!clipLineAgainstCam(C1, C2)) return;
    
    // Transform from camera space to screen coords
    ProjPos p1 = camToScreen(C1);
    ProjPos p2 = camToScreen(C2);

    // Only draw if within screen bounds
    if ((p1.x < 0 && p2.x < 0) || (p1.x > width && p2.x > width ) ||
        (p1.y < 0 && p2.y < 0) || (p1.y > height && p2.y > height)) {
            return;
        }

    SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);

}

// Draws an object (Wireframe or Filled)
void Renderer3D::drawObject(const Object* obj) {
    setDrawColor(obj->color);

    if (obj->wireframe) {
        for (Triangle tri : obj->tris) {
            drawLine(obj->vertices[tri.a], obj->vertices[tri.b]);
            drawLine(obj->vertices[tri.b], obj->vertices[tri.c]);
            drawLine(obj->vertices[tri.c], obj->vertices[tri.a]);
        }
    } else {
        geometryObjectFill(obj);
    }
}

void Renderer3D::drawCrosshair() {
    drawLine(cam.target - Vec3f(0.2f, 0, 0), cam.target + Vec3f(0.2f, 0, 0));
    drawLine(cam.target - Vec3f(0, 0.2f, 0), cam.target + Vec3f(0, 0.2f, 0));
    drawLine(cam.target - Vec3f(0, 0, 0.2f), cam.target + Vec3f(0, 0, 0.2f));
}

// Fill the triangles defining an object with clipping
void Renderer3D::geometryObjectFill(const Object* obj) {

    size_t numVerts = obj->vertices.size();

    struct RenderTri { int v[3]; Vec3f Acam; Vec3f Bcam; Vec3f Ccam; float depth; };
    std::vector<Vec3f> vertexNormals(numVerts, Vec3f());
    std::vector<int> amm(numVerts, 0);

    std::vector<RenderTri> renderList;
    for (auto &t : obj->tris) {
        Vec3f A = obj->vertices[t.a];
        Vec3f B = obj->vertices[t.b];
        Vec3f C = obj->vertices[t.c];

        Vec3f v = C - A;
        Vec3f Norm = (B - A).cross(v).normalize();
        for (int idx : {t.a, t.b, t.c}) {
            vertexNormals[idx] = vertexNormals[idx] + Norm;
            amm[idx]++;
        }

        Vec3f Acam = worldToCam(A);
        Vec3f Bcam = worldToCam(B);
        Vec3f Ccam = worldToCam(C);

        float z = (Acam.z + Bcam.z + Ccam.z) / 3.0f;
        renderList.push_back({{t.a, t.b, t.c}, Acam, Bcam, Ccam, z});
    }

    for (size_t i = 0; i < numVerts; i++)
        vertexNormals[i] = (vertexNormals[i] * (1 / static_cast<float>(amm[i]))).normalize();

    std::sort(renderList.begin(), renderList.end(),
        [](auto &L, auto &R){ return L.depth  > R.depth; });

    Vec3f lightDir_cam = worldToCam(lightDir).normalize();

    SDL_Vertex verts[3];
    for (RenderTri tri : renderList) {
        auto tris = clipAgainstNearPlane(tri.Acam, tri.Bcam, tri.Ccam);
        for (auto &t : tris) {
            std::array<ProjPos, 3> ndc;
            for (int i = 0; i < 3; i++)
                ndc[i] = camToScreen(t[i]);

            if (allVertsOutside(ndc)) continue;

            for (int i = 0; i < 3; i++) {

                int vi = tri.v[i];
                Vec3f imm = worldToCam(vertexNormals[vi]).normalize();

                // Testing zone for shading

                float hl = 0.5f * imm.dot(lightDir_cam) + 0.5f;

                Vec3f V = (cam.camPos - t[i]).normalize();
                Vec3f H = (lightDir_cam + V).normalize();
                float specular = powf(std::max(0.0f, imm.dot(H)), 16);

                float ambient = 0.28f;
                float diffWeight = 0.27f;
                float specWeight = 0.48f;

                float diff = ambient + diffWeight * hl + specWeight * specular;

                //float diff = 0.4f + 0.3f * std::max(0.0f, imm.dot(lightDir_cam));
                diff = std::min(1.0f, diff);
                if (diff < 0) diff = 0.0f;

                SDL_Color color = {
                    static_cast<Uint8>(obj->color.r * diff),
                    static_cast<Uint8>(obj->color.g * diff),
                    static_cast<Uint8>(obj->color.b * diff),
                    255
                };

                verts[i].position = { float(ndc[i].x), float(ndc[i].y) };
                verts[i].color = color;
                verts[i].tex_coord = {0, 0};
            }
    
            SDL_RenderGeometry(renderer, nullptr, verts, 3, nullptr, 0);
        }

    }
}

// Fill helper functions

// Helper function to validate triangle fill visibility
bool Renderer3D::allVertsOutside(std::array<ProjPos, 3> ndc) {
    bool allLeft = true, allRight = true, allBottom = true, allTop = true;

    for (auto &p : ndc) {
        allLeft &= (p.x < 0);
        allRight &= (p.x > width);
        allBottom &= (p.y < 0);
        allTop &= (p.y > height);
    }

    return allLeft || allRight || allBottom || allTop;
}

// Find the point between P and Q where z = nearPlane
static Vec3f intersectPlane(const Vec3f& P, const Vec3f&Q, float nearPlane) {
    float t = (nearPlane - P.z) / (Q.z - P.z);
    if (t > 1) t = 1.0f;
    if (t < 0) t = 0.0f;
    return P + (Q - P) * t;
}

// Clip the triangle ABC against the nearPlane defined by the camera
// Can return 0, 1, or 2 triangles
std::vector<std::array<Vec3f, 3>> Renderer3D::clipAgainstNearPlane(const Vec3f& A, const Vec3f& B, const Vec3f& C) {
    struct Vtx { Vec3f p; bool inside; };
    std::array<Vtx, 3> v = {{
        {A, A.z >= cam.near},
        {B, B.z >= cam.near},
        {C, C.z >= cam.near}
    }};

    int insideCount = (int) v[0].inside + (int) v[1].inside + (int) v[2].inside;
    std::vector<std::array<Vec3f, 3>> output;

    // If the entire triangle is clipped
    if (insideCount == 0)
        return output;
    else if (insideCount == 3) {
        // No clipping has occurred
        output.push_back({A, B, C});
        return output;
    }

    // Helper function adding a clipped vertex between vi and vj
    auto addIntersection = [&](int i, int j, std::vector<Vec3f>& verts) {
        verts.push_back(intersectPlane(v[i].p, v[j].p, cam.near));
    };

    // Build list of points for the clipped polygon
    std::vector<Vec3f> clippedVerts;
    for (int i = 0; i < 3; i++) {
        // ni means next i
        int ni = (i + 1) % 3;
        if (v[i].inside)
            clippedVerts.push_back(v[i].p);
        // If edge corsses the plane, add intersection
        if (v[i].inside != v[ni].inside)
            addIntersection(i, ni, clippedVerts);
    }

    // One Trangle
    if (clippedVerts.size() == 3)
        output.push_back({ clippedVerts[0], clippedVerts[1], clippedVerts[2] });
    else if (clippedVerts.size() == 4) {
        // Two Triangles
        // quad: verts 0-1-2-3
        // make two triangles: (0,1,2) and (0,2,3)
        output.push_back({ clippedVerts[0], clippedVerts[1], clippedVerts[2] });
        output.push_back({ clippedVerts[0], clippedVerts[2], clippedVerts[3] });
    }

    return output;
    
}

// Helper function to set Draw Color directly
void Renderer3D::setDrawColor(SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}


// PROJECTION FUNCTIONS


// Transform world space to camera space
Vec3f Renderer3D::worldToCam(const Vec3f& point) {
    Vec3f toPoint = point - cam.camPos;
    float camX = toPoint.dot(cam.right);
    float camY = toPoint.dot(cam.up);
    float camZ = toPoint.dot(cam.forward);  

    return Vec3f(camX, camY, camZ);   
}

// Convert from cam coords to screen coords
ProjPos Renderer3D::camToScreen(const Vec3f& C) {
    float x_proj = (cam.focalLength / C.z) * C.x;
    float y_proj = (cam.focalLength / C.z) * C.y;
    int x = static_cast<int>(x_proj * scale + width * 0.5f);
    int y = static_cast<int>(-y_proj * scale + height * 0.5f);
    return ProjPos(x, y);
}

// Clip a line from point A to point B if necessary
bool Renderer3D::clipLineAgainstCam(Vec3f& A, Vec3f& B) {
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


// CAMERA/RENDERER FUNCTIONS


// Rotate the camera based on mouse motion
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

// Move the camera along the screen based on mouse motion
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


// INPUT CAPTURE FUNCTIONS


// Capture keyboard input
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

// returns an enum based on key press
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