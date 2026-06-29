CXX = g++
NVCC = nvcc
CXXFLAGS = -O3 -march=native -fopenmp -std=c++17 -Wall -Wextra
CUDAFLAGS = -O3 -arch=sm_86 -std=c++17 --compiler-options -fopenmp
LDFLAGS = -lglfw -lGLEW -lGL -lX11 -lpthread -ldl -lOpenGL -lcudart -lstdc++

SRC = src/main.cpp src/core/engine.cpp src/classical/classical_engine.cpp \
      src/hybrid/hybrid_engine.cpp src/visualization/opengl_renderer.cpp \
      src/visualization/imgui_renderer.cpp
CUDA_SRC = src/quantum/quantum_engine.cu src/quantum/nbody_cuda.cu
IMGUI_SRC = $(wildcard external/imgui/*.cpp)

OBJ = $(SRC:.cpp=.o) $(CUDA_SRC:.cu=.o) $(IMGUI_SRC:.cpp=.o)
TARGET = quantum_engine

all: $(TARGET)

$(TARGET): $(OBJ)
$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cu
$(NVCC) $(CUDAFLAGS) -c $< -o $@

clean:
rm -f $(OBJ) $(TARGET)

.PHONY: all clean
