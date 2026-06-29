  """
Vizualizačný modul pre Quantum Engine
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.colors import LinearSegmentedColormap
from mpl_toolkits.mplot3d import Axes3D
from typing import Optional, Tuple, List
import time

class Visualizer:
    """Live 3D vizualizácia simulácie"""
    
    def __init__(self, engine, figsize: Tuple[int, int] = (14, 8)):
        """
        Inicializácia vizualizátora
        
        Args:
            engine: Inštancia QuantumEngine
            figsize: Veľkosť okna (šírka, výška)
        """
        self.engine = engine
        self.figsize = figsize
        self.fig = None
        self.ax1 = None  # 3D klasické častice
        self.ax2 = None  # Kvantová vlnová funkcia
        self.ax3 = None  # Štatistiky
        
        self.scatter = None
        self.line = None
        self.energy_line = None
        
        self.is_initialized = False
        self.frame_count = 0
        self.start_time = time.time()
        
        # Farba mapy pre kvantovú vlnu
        self.colors = plt.cm.viridis
        
        # Štatistiky pre grafy
        self.energy_history = []
        self.time_history = []
        self.max_history = 1000
    
    def initialize(self):
        """Inicializuje matplotlib figúry"""
        if self.is_initialized:
            return
        
        plt.ion()  # Interaktívny režim
        self.fig = plt.figure(figsize=self.figsize)
        
        # 1. 3D plot pre klasické častice
        self.ax1 = self.fig.add_subplot(231, projection='3d')
        self.ax1.set_title('🌌 Klasické častice (N-body)')
        self.ax1.set_xlabel('X')
        self.ax1.set_ylabel('Y')
        self.ax1.set_zlabel('Z')
        
        # 2. Kvantová vlnová funkcia
        self.ax2 = self.fig.add_subplot(232)
        self.ax2.set_title('⚛️ Kvantová vlnová funkcia')
        self.ax2.set_xlabel('x')
        self.ax2.set_ylabel('Pravdepodobnosť |ψ|²')
        self.ax2.grid(True, alpha=0.3)
        
        # 3. Štatistiky
        self.ax3 = self.fig.add_subplot(233)
        self.ax3.set_title('📊 Energia systému')
        self.ax3.set_xlabel('Čas (s)')
        self.ax3.set_ylabel('Energia')
        self.ax3.grid(True, alpha=0.3)
        
        # 4. Potenciál
        self.ax4 = self.fig.add_subplot(234)
        self.ax4.set_title('🔮 Potenciál V(x)')
        self.ax4.set_xlabel('x')
        self.ax4.set_ylabel('V(x)')
        self.ax4.grid(True, alpha=0.3)
        
        # 5. Kvantová fáza
        self.ax5 = self.fig.add_subplot(235)
        self.ax5.set_title('🌀 Kvantová fáza')
        self.ax5.set_xlabel('x')
        self.ax5.set_ylabel('Fáza (rad)')
        self.ax5.grid(True, alpha=0.3)
        
        # 6. Hybridné prepojenie
        self.ax6 = self.fig.add_subplot(236)
        self.ax6.set_title('🔗 Hybridné prepojenie')
        self.ax6.set_xlabel('Čas (s)')
        self.ax6.set_ylabel('Coupling')
        self.ax6.grid(True, alpha=0.3)
        
        plt.tight_layout()
        self.is_initialized = True
        
        print("✅ Vizualizácia inicializovaná")
    
    def update(self, frame: int):
        """
        Aktualizuje všetky grafy
        
        Args:
            frame: Číslo snímku
        """
        if not self.is_initialized:
            return
        
        # Spustenie niekoľkých simulačných krokov
        steps_per_frame = self.engine.config.get('steps_per_frame', 10)
        for _ in range(steps_per_frame):
            if self.engine.is_running():
                self.engine.step()
        
        # Získanie dát
        positions = self.engine.get_positions()
        wavefunc = self.engine.get_wavefunction()
        probability = self.engine.get_probability_density()
        current_time = self.engine.get_current_time()
        
        if len(positions) == 0 or len(wavefunc) == 0:
            return
        
        # 1. 3D častice
        self.ax1.clear()
        if len(positions) > 0:
            # Náhodný výber pre rýchlosť (max 5000 častíc)
            sample_size = min(5000, len(positions))
            indices = np.random.choice(len(positions), sample_size, replace=False)
            sample_pos = positions[indices]
            
            self.ax1.scatter(
                sample_pos[:, 0], sample_pos[:, 1], sample_pos[:, 2],
                c='cyan', s=1, alpha=0.6, marker='o'
            )
            
            # Nastavenie limitov
            grid_size = self.engine.config['grid_size'] / 2
            self.ax1.set_xlim(-grid_size, grid_size)
            self.ax1.set_ylim(-grid_size, grid_size)
            self.ax1.set_zlim(-grid_size, grid_size)
        
        self.ax1.set_title(f'🌌 Častice: {len(positions)} | t = {current_time:.2f}s')
        
        # 2. Kvantová vlnová funkcia
        self.ax2.clear()
        if len(probability) > 0:
            x = np.linspace(-5, 5, len(probability))
            self.ax2.fill_between(x, 0, probability, alpha=0.5, color='blue')
            self.ax2.plot(x, probability, 'b-', linewidth=2)
            self.ax2.set_ylim(0, max(probability) * 1.2 if max(probability) > 0 else 1)
            self.ax2.set_xlabel('x')
            self.ax2.set_ylabel('|ψ|²')
            self.ax2.grid(True, alpha=0.3)
        
        # 3. Energia
        self.ax3.clear()
        if len(self.energy_history) > 0:
            times = self.time_history[-len(self.energy_history):]
            self.ax3.plot(times, self.energy_history, 'r-', linewidth=2)
            self.ax3.set_xlabel('Čas (s)')
            self.ax3.set_ylabel('Energia')
            self.ax3.grid(True, alpha=0.3)
            self.ax3.legend(['Celková energia'])
        
        # 4. Potenciál
        self.ax4.clear()
        # TODO: Získať potenciál z enginu
        self.ax4.text(0.5, 0.5, 'Potenciál V(x)', 
                     horizontalalignment='center',
                     verticalalignment='center',
                     transform=self.ax4.transAxes)
        
        # 5. Kvantová fáza
        self.ax5.clear()
        if len(wavefunc) > 0:
            phase = np.arctan2(wavefunc[:, 1], wavefunc[:, 0])
            x = np.linspace(-5, 5, len(phase))
            self.ax5.plot(x, phase, 'g-', linewidth=2)
            self.ax5.set_xlabel('x')
            self.ax5.set_ylabel('Fáza (rad)')
            self.ax5.grid(True, alpha=0.3)
        
        # 6. Hybridné prepojenie
        self.ax6.clear()
        self.ax6.text(0.5, 0.5, f'Coupling: {self.engine.config["coupling_strength"]:.3f}',
                     horizontalalignment='center',
                     verticalalignment='center',
                     transform=self.ax6.transAxes)
        
        plt.tight_layout()
        self.frame_count += 1
        
        # Aktualizácia štatistík
        # TODO: Získať energiu z enginu
        # self.energy_history.append(energy)
        # self.time_history.append(current_time)
        
        # Obmedzenie histórie
        if len(self.energy_history) > self.max_history:
            self.energy_history = self.energy_history[-self.max_history:]
            self.time_history = self.time_history[-self.max_history:]
    
    def show(self, interval: int = 50, steps: Optional[int] = None):
        """
        Zobrazí animáciu
        
        Args:
            interval: Interval medzi snímkami (ms)
            steps: Počet krokov (None = nekonečne)
        """
        if not self.is_initialized:
            self.initialize()
        
        def update_frame(frame):
            self.update(frame)
            return []
        
        # Animácia
        self.anim = FuncAnimation(
            self.fig, update_frame,
            frames=steps if steps else None,
            interval=interval,
            blit=False,
            repeat=steps is None
        )
        
        plt.show(block=True)
    
    def save_frame(self, filename: str):
        """Uloží aktuálny snímok"""
        if self.is_initialized:
            self.fig.savefig(filename, dpi=150, bbox_inches='tight')
            print(f"💾 Snímok uložený: {filename}")
    
    def close(self):
        """Zatvorí vizualizáciu"""
        if self.is_initialized:
            plt.close(self.fig)
            self.is_initialized = False
            print("🔄 Vizualizácia zatvorená")


class SimpleVisualizer:
    """Jednoduchá vizualizácia pre rýchle testy"""
    
    def __init__(self, engine):
        self.engine = engine
        self.fig, (self.ax1, self.ax2) = plt.subplots(1, 2, figsize=(12, 5))
        self.scatter = None
        self.line = None
    
    def update(self):
        """Aktualizuje grafy"""
        positions = self.engine.get_positions()
        probability = self.engine.get_probability_density()
        
        # 3D častice
        self.ax1.clear()
        if len(positions) > 0:
            sample_size = min(2000, len(positions))
            indices = np.random.choice(len(positions), sample_size, replace=False)
            sample_pos = positions[indices]
            
            self.ax1.scatter(sample_pos[:, 0], sample_pos[:, 1], 
                           c='blue', s=1, alpha=0.5)
            self.ax1.set_title(f'Častice: {len(positions)}')
            self.ax1.set_xlabel('X')
            self.ax1.set_ylabel('Y')
            self.ax1.grid(True, alpha=0.3)
        
        # Kvantová vlna
        self.ax2.clear()
        if len(probability) > 0:
            x = np.linspace(-5, 5, len(probability))
            self.ax2.plot(x, probability, 'r-', linewidth=2)
            self.ax2.set_title('Kvantová pravdepodobnosť')
            self.ax2.set_xlabel('x')
            self.ax2.set_ylabel('|ψ|²')
            self.ax2.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.pause(0.01)
    
    def show(self):
        """Zobrazí vizualizáciu"""
        plt.ion()
        while self.engine.is_running():
            self.update()
        plt.ioff()
        plt.show()
