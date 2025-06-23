#include "JSONReader.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>

Simulation::Simulation(char* filename) {
    std::ifstream JSON(filename);

    if (!JSON.is_open()) {
        loadedSuccess = false;
        throw std::runtime_error("Error opening file!");
        return;
    }

    std::string line;

    // Skip opening {
    std::getline(JSON, line);

    // Read until the end of the file
    while (std::getline(JSON, line)) {
        // Search for definer in line
        std::size_t i = line.find(':');

        if (i != std::string::npos) {

            std::string memberName = retrieveString(line, 0);
            if (memberName.compare("name") == 0) {
                try {
                    std::string _name = retrieveString(line, i);
                    this->name.assign(_name);
                } catch (...) {
                    throw std::runtime_error("Error on: " + line);
                }
            } else if (memberName.compare("window") == 0) {
                std::getline(JSON, line);
                while (line.find('}') == std::string::npos) {
                    i = line.find(":");
                    if (i != std::string::npos) {
                        
                        try {
                            memberName = retrieveString(line, 0);
                            setWindowParam(line, memberName, i);
                        }
                        catch (const std::exception& e) {
                            loadedSuccess = false;
                            throw;
                        }
                    }
                    std::getline(JSON, line);
                }
            } else if (memberName.compare("camera") == 0) {
                std::getline(JSON, line);
                while (line.find('}') == std::string::npos) {
                    i = line.find(':');
                    if (i != std::string::npos) {
                        try {
                            memberName = retrieveString(line, 0);
                            setCameraParam(line, memberName, i);
                        }
                        catch (const std::exception& e) {
                            loadedSuccess = false;
                            throw;
                        }
                    }
                    std::getline(JSON, line);
                }
            } else if (memberName.compare("lighting") == 0) {
                std::getline(JSON, line);
                while (line.find('}') == std::string::npos) {
                    i = line.find(':');
                    if (i != std::string::npos) {
                        try {
                            memberName = retrieveString(line, 0);
                            setLightingParam(line, memberName, i);
                        }
                        catch (const std::exception& e) {
                            loadedSuccess = false;
                            throw;
                        }
                    }
                    std::getline(JSON, line);
                }
            } else if (memberName.compare("objects") == 0) {
                // The closing bracket for the objects array will not be on the same line as an opening bracket
                while (true) {

                    if (line.find(']') != std::string::npos)
                        if (line.find('[') == std::string::npos)
                            break;

                    std::getline(JSON, line);
                    // Find start of one object
                    if (line.find('{') != std::string::npos) {
                        // Until we reach the end of the object
                        ObjectModel objM;
                        std::getline(JSON, line);
                        while (line.find('}') == std::string::npos) {
                            i = line.find(':');
                            if (i != std::string::npos) {
                                try {
                                    memberName = retrieveString(line, 0);
                                    setObjectParam(objM, line, memberName, i);
                                }
                                catch (const std::exception& e) {
                                    loadedSuccess = false;
                                    throw;
                                }
                            }
                            std::getline(JSON, line);
                        }
                        this->objModels.push_back(objM);
                    }
                }
            } else if (memberName.compare("fields") == 0) {
                // The closing bracket for the objects array will not be on the same line as an opening bracket
                while (true) {

                    if (line.find(']') != std::string::npos)
                        if (line.find('[') == std::string::npos)
                            break;

                    std::getline(JSON, line);
                    // Find start of one object
                    if (line.find('{') != std::string::npos) {
                        // Until we reach the end of the object
                        FieldModel fieldM;
                        std::getline(JSON, line);
                        while (line.find('}') == std::string::npos) {
                            i = line.find(':');
                            if (i != std::string::npos) {
                                try {
                                    memberName = retrieveString(line, 0);
                                    setFieldParam(fieldM, line, memberName, i);
                                }
                                catch (const std::exception& e) {
                                    loadedSuccess = false;
                                    throw;
                                }
                            }
                            std::getline(JSON, line);
                        }
                        this->fieldModels.push_back(fieldM);
                    }
                }
            } else {
                std::cerr << "Invalid Parameter Name Given!:\n" + line << std::endl;
                loadedSuccess = false;
                return;
            }

        }
    }

    JSON.close();
    loadedSuccess = true;
}

void Simulation::displaySim() {
    std::cout << "name: " + name << std::endl;
    std::cout << "| Window |" << std::endl;
    std::cout << "width: " + std::to_string(width) << std::endl;
    std::cout << "height: " + std::to_string(height) << std::endl;
    std::cout << "scale: " + std::to_string(scale) << std::endl;
    std::cout << "lightDir: " + lightDir.toString() << std::endl;
    std::cout << "| Camera |" << std::endl;
    std::cout << "distance: " + std::to_string(distance) << std::endl;
    std::cout << "rotateSpeed: " + std::to_string(rotateSpeed) << std::endl;
    std::cout << "panSpeed: " + std::to_string(panSpeed) << std::endl;
    std::cout << "zoomSpeed: " + std::to_string(zoomSpeed) << std::endl;
    std::cout << "focalLength: " + std::to_string(focalLength) << std::endl;
    std::cout << "near: " + std::to_string(near) << std::endl;
    std::cout << "| Objects |" << std::endl;

    for (ObjectModel objM : objModels) {
        std::cout << "name: " + objM.name << std::endl;
        std::cout << "shape: " + std::to_string(objM.shape) << std::endl;
        std::cout << "center: " + objM.center.toString() << std::endl;
        std::cout << "radius: " + std::to_string(objM.radius) << std::endl;
        std::cout << "color: " + objM.color.toString() << std::endl;
        std::cout << std::boolalpha << "wireframe: " + objM.wireframe << std::endl;
        std::cout << "mass: " + std::to_string(objM.mass) << std::endl;
        std::cout << "charge: " + std::to_string(objM.charge) << std::endl;
        std::cout << "initialVelocity: " + objM.initVelocity.toString() << std::endl;
        std::cout << "initialAccel: " + objM.initAccel.toString() << std::endl;
        std::cout << "initAngVelocity: " + objM.initAngVelocity.toString() << std::endl;
        std::cout << "initAngAccel: " + objM.initAngAccel.toString() << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
}

std::string Simulation::retrieveString(std::string line, int pos) {
    int i = line.find('\"', pos);
    std::string::iterator it = line.begin() + i + 1;
    std::string str;
    while (*it != '\"') {
        str.append(1, *it);
        it++;
    }
    return str;
}

bool Simulation::retrieveBool(std::string line) {
    if (line.find("true") != std::string::npos)
        return true;
    else
        return false;
}

float Simulation::retrieveFloat(std::string line, int pos) {
    std::string::iterator it = line.begin() + pos;
    // Walk until finding first digit
    while (!isdigit(*it) && *it != '-') {
        it++;
        pos++;
    }
    int start = pos;
    while(it != line.end() && *it != ',') {
        it++;
        pos++;
    }
    std::string num = line.substr(start, pos-1);
    return (std::stof(num));
}

Vec3f Simulation::retrieveVector(std::string line) {
    int start = line.find('[');
    int end = line.find(']');
    std::string seq = line.substr(start + 1, end - 1);

    std::stringstream ss(seq);
    Vec3f ret = Vec3f();
    std::string hold;
    std::getline(ss, hold, ',');
    ret.x = std::stof(hold);
    std::getline(ss, hold, ',');
    ret.y = std::stof(hold);
    std::getline(ss, hold, ',');
    ret.z = std::stof(hold);
    return ret;
}

ObjectModel::Color Simulation::retrieveColor(std::string line) {
    int start = line.find('[');
    int end = line.find(']');
    std::string seq = line.substr(start + 1, end - 1);

    std::stringstream ss(seq);
    ObjectModel::Color ret;
    std::string hold;
    std::getline(ss, hold, ',');
    ret.r = static_cast<uint8_t>(std::stoi(hold));
    std::getline(ss, hold, ',');
    ret.g = static_cast<uint8_t>(std::stoi(hold));
    std::getline(ss, hold, ',');
    ret.b = static_cast<uint8_t>(std::stoi(hold));
    std::getline(ss, hold, ',');
    ret.a = static_cast<uint8_t>(std::stoi(hold)); 
    return ret;
}

void Simulation::setWindowParam(std::string line, std::string memberName, int pos) {
    try {

        if (memberName.compare("width") == 0) 
            this->width = static_cast<int>(retrieveFloat(line, pos));
        else if (memberName.compare("height") == 0)
            this->height = static_cast<int>(retrieveFloat(line, pos));
        else if (memberName.compare("scale") == 0)
            this->scale = retrieveFloat(line, pos);
        else
            throw 500;
    } catch (...) {
        throw std::runtime_error("Error on: " + line);
    }
    
}

void Simulation::setCameraParam(std::string line, std::string memberName, int pos) {    
    try {
        float param = retrieveFloat(line, pos);
        if (memberName.compare("distance") == 0)
            this->distance = param;
        else if (memberName.compare("rotate_speed") == 0)
            this->rotateSpeed = param;
        else if (memberName.compare("pan_speed") == 0)
            this->panSpeed = param;
        else if (memberName.compare("zoom_speed") == 0)
            this->zoomSpeed = param;
        else if (memberName.compare("focal_length") == 0)
            this->focalLength = param;
        else if (memberName.compare("near") == 0)
            this->near = param;
        else
            throw 500;
    } catch (...) {
        throw std::runtime_error("Error on: " + line);
    }
}

void Simulation::setObjectParam(ObjectModel& objM, std::string line, std::string memberName, int pos) {
    try {
        if (memberName.compare("name") == 0)
            objM.name.assign(retrieveString(line, pos));
        else if (memberName.compare("shape") == 0) {
            std::string _shape = retrieveString(line, pos);
            // Could use map if number of shapes increases
            if (_shape.compare("sphere") == 0)
                objM.shape = ObjectModel::Shape::SPHERE;
            else if (_shape.compare("tetrahedron") == 0)
                objM.shape = ObjectModel::Shape::TETRAHEDRON;
            else if (_shape.compare("icosahedron") == 0)
                objM.shape = ObjectModel::Shape::ICOSAHEDRON;
            else
                throw 500;
        }
        else if (memberName.compare("center") == 0)
            objM.center = retrieveVector(line);
        else if (memberName.compare("radius") == 0)
            objM.radius = retrieveFloat(line, pos);
        else if (memberName.compare("color") == 0)
            objM.color = retrieveColor(line);
        else if (memberName.compare("wireframe") == 0)
            objM.wireframe = retrieveBool(line);
        else if (memberName.compare("mass") == 0)
            objM.mass = retrieveFloat(line, pos);
        else if (memberName.compare("charge") == 0)
            objM.charge = retrieveFloat(line, pos);
        else if (memberName.compare("initial_velocity") == 0)
            objM.initVelocity = retrieveVector(line);
        else if (memberName.compare("initial_acceleration") == 0)
            objM.initAccel = retrieveVector(line);
        else if (memberName.compare("initial_angular_velocity") == 0)
            objM.initAngVelocity = retrieveVector(line);
        else if (memberName.compare("initial_angular_acceleration") == 0)
            objM.initAngAccel = retrieveVector(line);
        else
           throw 500;
    } catch (...) {
        throw std::runtime_error("Error on: " + line);
    }
}

void Simulation::setLightingParam(std::string line, std::string memberName, int pos) {
    try {

        if (memberName.compare("light_dir") == 0) 
            this->lightDir = retrieveVector(line);
        else if (memberName.compare("ambient") == 0)
            this->ambient = retrieveFloat(line, pos);
        else if (memberName.compare("diff_weight") == 0)
            this->diffWeight = retrieveFloat(line, pos);
        else if (memberName.compare("spec_weight") == 0)
            this->specWeight = retrieveFloat(line, pos);
        else if (memberName.compare("shininess") == 0)
            this->shininess = retrieveFloat(line, pos);
        else if (memberName.compare("gamma") == 0)
            this->gamma = retrieveFloat(line, pos);
        else
            throw 500;
    } catch (...) {
        throw std::runtime_error("Error on: " + line);
    }
    
}

void Simulation::setFieldParam(FieldModel& fieldM, std::string line, std::string memberName, int pos) {
    try {
        if (memberName.compare("name") == 0) 
            fieldM.name.assign(retrieveString(line, pos));
        else if (memberName.compare("type") == 0) {
            std::string _type = retrieveString(line, pos);
            if (_type.compare("electric") == 0)
                fieldM.type = FieldModel::Type::Electric;
            else if (_type.compare("magentic") == 0)
                fieldM.type = FieldModel::Type::Magnetic;
            else
                throw 500;
        }
        else if (memberName.compare("direction") == 0)
            fieldM.direction = retrieveVector(line);
        else if (memberName.compare("strength") == 0)
            fieldM.strength = retrieveFloat(line, pos);
        else
            throw 500;
    } catch (...) {
        throw std::runtime_error("Error on: " + line);
    }
}