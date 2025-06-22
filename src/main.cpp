#include <iostream>
#include <SDL.h>

#include "Renderer.h"
#include "JSONReader.h"

int main(int argc, char* argv[]) {
    
    // Validate Correct Arguments
    if (argc != 2) {
        std::cerr << "Usage ./progname scene.json" << std::endl;
        return 1;
    }

    // Read JSON file and validate a successful read and load
    Simulation* sim = nullptr;
    try {
        sim = new Simulation(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        delete sim;
        return 1;
    } catch (...) {
        std::cerr << "An unknown error occurred while loading scene file..." << std::endl;
        delete sim;
        return 1;
    }

    if (!sim->loadedSuccess) {
        std::cerr << "Could not load scene successfully!" << std::endl;
        delete sim;
        return 1;
    }

    // => process JSON file into Simulation class

    // => pass map/class to renderer for usage
    Renderer3D renderer(800, 600);
    renderer.run();
    return 0;
}
