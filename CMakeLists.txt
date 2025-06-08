cmake_minimum_required(VERSION 3.10)
project(EMsim VERSION 0.1 LANGUAGES CXX)

# 1) Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SDL2_DIR "C:/tools/SDL2-2.32.8/cmake")

# 2) Find SDL2 (make sure SDL2_DIR is set if not found automatically)
#    On Windows, later we’ll pass -DSDL2_DIR="C:/Libraries/SDL2-2.26.5/lib/cmake/SDL2"
find_package(SDL2 REQUIRED)

if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found. "
        "Pass -DSDL2_DIR=<path-to-/lib/cmake/SDL2> to CMake.")
endif()

# 3) Include SDL2’s headers (SDL2_INCLUDE_DIRS points to ".../include")
include_directories(${SDL2_INCLUDE_DIRS})

# 4) Also include our own headers (include/)
include_directories(${CMAKE_SOURCE_DIR}/include)

# 5) Add our executable and its sources
add_executable(EMsim
    src/main.cpp
    src/Renderer.cpp
    src/PhysicsEngine.cpp
)

# 6) Link against SDL2 (SDL2_LIBRARIES comes from find_package)
target_link_libraries(EMsim PRIVATE ${SDL2_LIBRARIES})

# 7) Copy SDL2.dll next to the EXE after build (Windows)
#    SDL2_DIR is like "C:/Libraries/SDL2-2.26.5/lib/cmake/SDL2", so
#    SDL2_DLL points to "C:/Libraries/SDL2-2.26.5/bin/SDL2.dll"
set(SDL2_DLL "${SDL2_DIR}/../lib/x64/SDL2.dll")
add_custom_command(TARGET EMsim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DLL}"
        $<TARGET_FILE_DIR:EMsim>
)
