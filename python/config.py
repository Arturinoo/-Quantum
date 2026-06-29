  """
Konfiguračný modul pre Quantum Engine
"""

import yaml
import json
from pathlib import Path
from typing import Dict, Any

class Config:
    """Trieda pre správu konfigurácie"""
    
    def __init__(self, config_dict: Dict[str, Any] = None):
        self.data = config_dict or self._default_config()
    
    def _default_config(self) -> Dict[str, Any]:
        """Predvolená konfigurácia"""
        return {
            # Časové nastavenia
            'dt': 0.001,
            'total_time': 10.0,
            'steps_per_frame': 10,
            
            # Klasický engine
            'particle_count': 10000,
            'gravitational_constant': 1.0,
            'softening_factor': 0.01,
            'theta': 0.6,  # Barnes-Hut parameter
            
            # Kvantový engine
            'grid_points': 1024,
            'grid_size': 10.0,
            'mass': 1.0,
            'hbar': 1.0,
            
            # Potenciál
            'potential_barrier_height': 5.0,
            'potential_barrier_width': 1.0,
            'potential_barrier_position': 0.0,
            
            # Hybridné prepojenie
            'coupling_strength': 0.01,
            'enable_quantum_noise': True,
            'coupling_mode': 2,  # 0=žiadne, 1=jednosmerné, 2=obojsmerné
            
            # Vizualizácia
            'enable_visualization': True,
            'window_width': 1280,
            'window_height': 720,
            'background_color': [0.1, 0.1, 0.15],
            
            # Výkon
            'use_barnes_hut': True,
            'use_cuda': True,
            'num_threads': 8,  # Pre OpenMP
            
            # Výstup
            'save_snapshots': False,
            'snapshot_interval': 100,
            'output_dir': './data/snapshots/'
        }
    
    def __getitem__(self, key):
        return self.data[key]
    
    def __setitem__(self, key, value):
        self.data[key] = value
    
    def __contains__(self, key):
        return key in self.data
    
    def get(self, key, default=None):
        return self.data.get(key, default)
    
    def update(self, other_dict):
        self.data.update(other_dict)
    
    def save(self, filename: str):
        """Uloží konfiguráciu do YAML súboru"""
        path = Path(filename)
        path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(path, 'w') as f:
            yaml.dump(self.data, f, default_flow_style=False)
    
    @classmethod
    def load(cls, filename: str):
        """Načíta konfiguráciu z YAML súboru"""
        with open(filename, 'r') as f:
            data = yaml.safe_load(f)
        return cls(data)
    
    def to_dict(self) -> Dict[str, Any]:
        """Vráti konfiguráciu ako slovník"""
        return self.data.copy()


def load_config(filename: str) -> Config:
    """Pomocná funkcia na načítanie konfigurácie"""
    return Config.load(filename)


def save_config(config: Config, filename: str):
    """Pomocná funkcia na uloženie konfigurácie"""
    config.save(filename)
