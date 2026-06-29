  """
Hlavný Python wrapper pre Quantum Engine
"""

import ctypes
import numpy as np
from pathlib import Path
from typing import Optional, Tuple, List
import os
import sys

from .config import Config, load_config

class QuantumEngine:
    """Python wrapper pre C++/CUDA Quantum Engine"""
    
    def __init__(self, config_file: Optional[str] = None):
        """
        Inicializácia enginu
        
        Args:
            config_file: Cesta ku konfiguračnému súboru (YAML)
        """
        self.lib = None
        self.config = Config()
        self._initialized = False
        self._particle_positions = None
        self._wavefunction = None
        
        # Načítanie konfigurácie
        if config_file:
            self.config = load_config(config_file)
        
        # Načítanie knižnice
        self._load_library()
    
    def _load_library(self):
        """Načíta skompilovanú C++ knižnicu"""
        # Cesta ku knižnici
        lib_paths = [
            Path(__file__).parent.parent / "build" / "libquantum_engine.so",
            Path(__file__).parent.parent / "build" / "quantum_bindings.so",
            Path(__file__).parent.parent / "build" / "libquantum_bindings.so",
            Path("./build/libquantum_engine.so"),
            Path("./build/quantum_bindings.so"),
        ]
        
        # Pre Windows
        if sys.platform == 'win32':
            lib_paths = [p.with_suffix('.dll') for p in lib_paths]
        # Pre Mac
        elif sys.platform == 'darwin':
            lib_paths = [p.with_suffix('.dylib') for p in lib_paths]
        
        lib_path = None
        for path in lib_paths:
            if path.exists():
                lib_path = path
                break
        
        if lib_path is None:
            raise FileNotFoundError(
                "Knižnica Quantum Engine nebola nájdená! "
                "Spusti najprv 'scripts/build.sh'"
            )
        
        print(f"📚 Načítavam knižnicu: {lib_path}")
        self.lib = ctypes.CDLL(str(lib_path))
        
        # Definícia funkcií
        self._setup_functions()
    
    def _setup_functions(self):
        """Nastaví typy argumentov pre C++ funkcie"""
        lib = self.lib
        
        # engine_init
        lib.engine_init.argtypes = [ctypes.c_void_p]
        lib.engine_init.restype = ctypes.c_bool
        
        # engine_step
        lib.engine_step.argtypes = []
        lib.engine_step.restype = ctypes.c_bool
        
        # engine_run
        lib.engine_run.argtypes = []
        lib.engine_run.restype = None
        
        # engine_get_positions
        lib.engine_get_positions.argtypes = [ctypes.POINTER(ctypes.c_float)]
        lib.engine_get_positions.restype = None
        
        # engine_get_wavefunction
        lib.engine_get_wavefunction.argtypes = [ctypes.POINTER(ctypes.c_float)]
        lib.engine_get_wavefunction.restype = None
        
        # engine_is_running
        lib.engine_is_running.argtypes = []
        lib.engine_is_running.restype = ctypes.c_bool
        
        # engine_get_current_time
        lib.engine_get_current_time.argtypes = []
        lib.engine_get_current_time.restype = ctypes.c_float
        
        # engine_add_potential_barrier
        lib.engine_add_potential_barrier.argtypes = [ctypes.c_float, ctypes.c_float, ctypes.c_float]
        lib.engine_add_potential_barrier.restype = None
        
        # engine_set_coupling_strength
        lib.engine_set_coupling_strength.argtypes = [ctypes.c_float]
        lib.engine_set_coupling_strength.restype = None
        
        # engine_shutdown
        lib.engine_shutdown.argtypes = []
        lib.engine_shutdown.restype = None
    
    def initialize(self) -> bool:
        """
        Inicializuje engine s aktuálnou konfiguráciou
        
        Returns:
            bool: True ak inicializácia prebehla úspešne
        """
        if not self.lib:
            return False
        
        # Konverzia konfigurácie na C++ štruktúru
        # TODO: Implementovať konverziu cez pybind11 alebo ctypes štruktúru
        
        # Zatiaľ len zavoláme init bez konfigurácie
        result = self.lib.engine_init(0)
        
        if result:
            self._initialized = True
            # Alokácia numpy polí
            particle_count = self.config['particle_count']
            grid_points = self.config['grid_points']
            
            self._particle_positions = np.zeros((particle_count, 3), dtype=np.float32)
            self._wavefunction = np.zeros((grid_points, 2), dtype=np.float32)
            
            print("✅ Engine inicializovaný!")
        else:
            print("❌ Chyba pri inicializácii enginu!")
        
        return result
    
    def step(self) -> bool:
        """
        Vykoná jeden simulačný krok
        
        Returns:
            bool: True ak krok prebehol úspešne
        """
        if not self._initialized:
            print("❌ Engine nie je inicializovaný!")
            return False
        
        return self.lib.engine_step()
    
    def run(self, steps: Optional[int] = None):
        """
        Spustí simuláciu
        
        Args:
            steps: Počet krokov (None = beží do dokončenia)
        """
        if not self._initialized:
            print("❌ Engine nie je inicializovaný!")
            return
        
        if steps is None:
            # Beží až do dokončenia
            self.lib.engine_run()
        else:
            # Beží zadaný počet krokov
            for i in range(steps):
                if not self.step():
                    print(f"❌ Simulácia zlyhala v kroku {i}")
                    break
                
                # Progress bar
                if i % 100 == 0:
                    progress = (i / steps) * 100
                    print(f"⏳ Progress: {progress:.1f}%", end='\r')
            
            print(f"\n✅ Simulácia dokončená: {steps} krokov")
    
    def get_positions(self) -> np.ndarray:
        """
        Získa polohy častíc
        
        Returns:
            np.ndarray: Array tvaru (N, 3) s pozíciami
        """
        if not self._initialized or self._particle_positions is None:
            return np.array([])
        
        ptr = self._particle_positions.ctypes.data_as(ctypes.POINTER(ctypes.c_float))
        self.lib.engine_get_positions(ptr)
        
        return self._particle_positions
    
    def get_wavefunction(self) -> np.ndarray:
        """
        Získa vlnovú funkciu
        
        Returns:
            np.ndarray: Array tvaru (N, 2) [real, imag]
        """
        if not self._initialized or self._wavefunction is None:
            return np.array([])
        
        ptr = self._wavefunction.ctypes.data_as(ctypes.POINTER(ctypes.c_float))
        self.lib.engine_get_wavefunction(ptr)
        
        return self._wavefunction
    
    def get_probability_density(self) -> np.ndarray:
        """
        Získa hustotu pravdepodobnosti |ψ|²
        
        Returns:
            np.ndarray: Array tvaru (N,)
        """
        wavefunc = self.get_wavefunction()
        if len(wavefunc) == 0:
            return np.array([])
        
        return wavefunc[:, 0]**2 + wavefunc[:, 1]**2
    
    def is_running(self) -> bool:
        """Vráti True ak simulácia beží"""
        return self.lib.engine_is_running()
    
    def get_current_time(self) -> float:
        """Vráti aktuálny simulačný čas"""
        return self.lib.engine_get_current_time()
    
    def add_potential_barrier(self, position: float, width: float, height: float):
        """
        Pridá potenciálovú bariéru pre kvantový engine
        
        Args:
            position: Stred bariéry
            width: Šírka bariéry
            height: Výška bariéry
        """
        if self._initialized:
            self.lib.engine_add_potential_barrier(
                ctypes.c_float(position),
                ctypes.c_float(width),
                ctypes.c_float(height)
            )
    
    def set_coupling_strength(self, strength: float):
        """
        Nastaví silu hybridného prepojenia
        
        Args:
            strength: Sila prepojenia (0-1)
        """
        if self._initialized:
            self.lib.engine_set_coupling_strength(ctypes.c_float(strength))
            self.config['coupling_strength'] = strength
    
    def shutdown(self):
        """Vypne engine a uvoľní zdroje"""
        if self._initialized and self.lib:
            self.lib.engine_shutdown()
            self._initialized = False
            print("🛑 Engine vypnutý")
    
    def __enter__(self):
        """Context manager - vstup"""
        self.initialize()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager - výstup"""
        self.shutdown()
    
    def __del__(self):
        """Destruktor"""
        self.shutdown()
