# 🚀 Quantum Engine

Hybridný Quantum-Classical simulačný engine s CUDA akceleráciou.

## 🎯 Features

- ⚛️ **Kvantová simulácia** - Schrödingerova rovnica na GPU (CUDA)
- 🌌 **Klasická N-body** - Gravitácia s Barnes-Hut optimalizáciou
- 🔗 **Hybridné prepojenie** - Kvantové a klasické systémy sa ovplyvňujú
- 🎨 **Live vizualizácia** - 3D zobrazenie častíc a vlnovej funkcie
- 🚀 **Vysoký výkon** - RTX 4060 + Ryzen 7 5800X

## 📦 Požiadavky

- **Hardvér:** NVIDIA GPU (RTX 4060 odporúčaná)
- **OS:** Linux / Windows (WSL2) / MacOS (bez CUDA)
- **Softvér:**
  - CUDA Toolkit 11+
  - CMake 3.15+
  - Python 3.8+
  - GCC/G++ 9+

## 🔧 Inštalácia

```bash
# 1. Klonovanie
git clone <repository>
cd quantum_engine

# 2. Inštalácia Python závislostí
pip install -r requirements.txt

# 3. Build
./scripts/build.sh

# 4. Spustenie
python python/examples/basic_simulation.py