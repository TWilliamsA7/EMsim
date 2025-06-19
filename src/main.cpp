#include <iostream>
#include <SDL.h>

#include "Renderer.h"

int main(int argc, char* argv[]) {
    
    Renderer3D renderer(800, 600);
    renderer.run();
    return 0;
}
