#!/bin/bash

echo "🔧 Building Quantum Engine..."

# Vytvorenie build priečinka
mkdir -p build_manual
cd build_manual

# Include paths
IMGUI_DIR="../external/imgui"
INCLUDE_FLAGS="-I$IMGUI_DIR -I../include -I../src"

# Kompilácia C++ súborov
echo "📦 Compiling C++ files..."
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c ../src/main.cpp -o main.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c ../src/core/engine.cpp -o engine.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c ../src/classical/classical_engine.cpp -o classical.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c ../src/hybrid/hybrid_engine.cpp -o hybrid.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c ../src/visualization/opengl_renderer.cpp -o opengl_renderer.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c ../src/visualization/imgui_renderer.cpp -o imgui_renderer.o

# Kompilácia ImGui
echo "📦 Compiling ImGui..."
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c $IMGUI_DIR/imgui.cpp -o imgui.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c $IMGUI_DIR/imgui_draw.cpp -o imgui_draw.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c $IMGUI_DIR/imgui_widgets.cpp -o imgui_widgets.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c $IMGUI_DIR/imgui_tables.cpp -o imgui_tables.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c $IMGUI_DIR/imgui_demo.cpp -o imgui_demo.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c $IMGUI_DIR/imgui_impl_glfw.cpp -o imgui_impl_glfw.o
g++ -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra $INCLUDE_FLAGS -c $IMGUI_DIR/imgui_impl_opengl3.cpp -o imgui_impl_opengl3.o

# Kompilácia CUDA súborov
echo "📦 Compiling CUDA files..."
nvcc -O3 -arch=sm_86 -std=c++17 --compiler-options -fopenmp $INCLUDE_FLAGS -c ../src/quantum/quantum_engine.cu -o quantum_engine.o
nvcc -O3 -arch=sm_86 -std=c++17 --compiler-options -fopenmp $INCLUDE_FLAGS -c ../src/quantum/nbody_cuda.cu -o nbody_cuda.o

# Linkovanie
echo "🔗 Linking..."
g++ -O3 -fopenmp -o quantum_engine \
    main.o engine.o classical.o hybrid.o opengl_renderer.o imgui_renderer.o \
    imgui.o imgui_draw.o imgui_widgets.o imgui_tables.o imgui_demo.o \
    imgui_impl_glfw.o imgui_impl_opengl3.o \
    quantum_engine.o nbody_cuda.o \
    -lglfw -lGLEW -lGL -lX11 -lpthread -ldl -lOpenGL -lcudart -lstdc++

echo "✅ Build complete! Running..."
./quantum_engine
