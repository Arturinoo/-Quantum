  #!/bin/bash

echo "🔨 Quantum Engine - Build Script"
echo "================================"
echo ""

# Kontrola CUDA
if ! command -v nvcc &> /dev/null; then
    echo "❌ CUDA Toolkit nebol nájdený!"
    echo "   Nainštaluj CUDA: https://developer.nvidia.com/cuda-downloads"
    exit 1
fi

echo "✅ CUDA nájdené: $(nvcc --version | grep 'release' | awk '{print $6}')"

# Kontrola CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake nebol nájdený!"
    echo "   Nainštaluj CMake: sudo apt install cmake"
    exit 1
fi

echo "✅ CMake nájdené: $(cmake --version | head -n1 | awk '{print $3}')"
echo ""

# Vytvorenie build priečinka
echo "📁 Vytváram build priečinok..."
mkdir -p build
cd build

# CMake konfigurácia
echo "⚙️  CMake konfigurácia..."
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCUDA_ARCHITECTURES=86 \
         -DCMAKE_CXX_FLAGS="-O3 -march=native"

if [ $? -ne 0 ]; then
    echo "❌ CMake konfigurácia zlyhala!"
    exit 1
fi

# Kompilácia
echo "🔨 Kompilácia..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "❌ Kompilácia zlyhala!"
    exit 1
fi

echo "✅ Kompilácia úspešná!"

# Kopírovanie knižníc
echo "📦 Kopírujem knižnice..."
cp libquantum_bindings.so ../python/ 2>/dev/null || true
cp libquantum_engine.so ../python/ 2>/dev/null || true

cd ..

echo ""
echo "╔════════════════════════════════════════════╗"
echo "║     ✅ BUILD DOKONČENÝ!                  ║"
echo "╚════════════════════════════════════════════╝"
echo ""
echo "🚀 Spustenie:"
echo "   python python/examples/basic_simulation.py"
echo ""
echo "📚 Dokumentácia:"
echo "   docs/architecture.md"
echo ""
