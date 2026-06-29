cmake_minimum_required(VERSION 3.15)
project(QuantumEngine VERSION 1.3.0 LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

set(CMAKE_CUDA_ARCHITECTURES "86")

find_package(CUDA REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)

find_path(GLM_INCLUDE_DIR NAMES glm/glm.hpp)
find_library(GLFW_LIBRARY NAMES glfw3 glfw)
find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h)
find_package(GLEW REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)
file(GLOB IMGUI_BACKEND_SOURCES ${IMGUI_DIR}/imgui_impl_*.cpp)

add_executable(quantum_engine
    src/main.cpp
    src/core/engine.cpp
    src/classical/classical_engine.cpp
    src/quantum/quantum_engine.cu
    src/quantum/nbody_cuda.cu
    src/hybrid/hybrid_engine.cpp
    src/visualization/opengl_renderer.cpp
    src/visualization/imgui_renderer.cpp
    ${IMGUI_SOURCES}
    ${IMGUI_BACKEND_SOURCES}
)

target_link_libraries(quantum_engine
    ${CUDA_LIBRARIES}
    cudart_static
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY}
    GLEW
    GL
    OpenMP::OpenMP_CXX
)

target_include_directories(quantum_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${CUDA_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
)

target_compile_options(quantum_engine PRIVATE
    -O3
    -march=native
    -Wall
    -Wextra
    -fopenmp
)

# JEDNODUCHÉ CUDA FLAGY – BEZ -Xcompiler
target_compile_options(quantum_engine PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_86>
    $<$<COMPILE_LANGUAGE:CUDA>:-O3>
    $<$<COMPILE_LANGUAGE:CUDA>:--compiler-options -fopenmp>
)
