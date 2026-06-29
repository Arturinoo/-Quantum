  #!/usr/bin/env python3
"""
Základná simulácia s Quantum Engine
"""

import sys
import time
from pathlib import Path

# Pridanie cesty k projektu
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))

from python.engine import QuantumEngine
from python.visualization import Visualizer
from python.config import Config

def main():
    """Hlavná funkcia"""
    print("╔══════════════════════════════════════════════╗")
    print("║     🚀 QUANTUM ENGINE - Basic Simulation    ║")
    print("╚══════════════════════════════════════════════╝\n")
    
    # 1. Vytvorenie konfigurácie
    config = Config()
    config['particle_count'] = 5000
    config['grid_points'] = 512
    config['total_time'] = 5.0
    config['dt'] = 0.001
    config['coupling_strength'] = 0.01
    config['steps_per_frame'] = 10
    config['enable_visualization'] = True
    
    print("📋 Konfigurácia:")
    for key, value in config.data.items():
        print(f"   {key}: {value}")
    print()
    
    # 2. Inicializácia enginu
    print("🔧 Inicializácia enginu...")
    engine = QuantumEngine()
    engine.config = config
    
    if not engine.initialize():
        print("❌ Chyba pri inicializácii enginu!")
        return
    
    # 3. Pridanie potenciálových bariér
    print("🔮 Pridávam potenciálové bariéry...")
    engine.add_potential_barrier(-1.5, 0.5, 8.0)
    engine.add_potential_barrier(1.5, 0.5, 8.0)
    
    # 4. Vizualizácia
    print("🎨 Spúšťam vizualizáciu...")
    visualizer = Visualizer(engine)
    visualizer.initialize()
    
    # 5. Spustenie simulácie
    print("▶️  Spúšťam simuláciu...\n")
    start_time = time.time()
    
    try:
        visualizer.show(interval=50, steps=500)
    except KeyboardInterrupt:
        print("\n⏹️  Simulácia prerušená užívateľom")
    finally:
        # 6. Uloženie štatistík
        end_time = time.time()
        elapsed = end_time - start_time
        
        print(f"\n📊 Štatistiky:")
        print(f"   Celkový čas simulácie: {engine.get_current_time():.2f} s")
        print(f"   Reálny čas: {elapsed:.2f} s")
        print(f"   Počet krokov: {engine.get_step_count() if hasattr(engine, 'get_step_count') else 'N/A'}")
        print(f"   Stav: {'Beží' if engine.is_running() else 'Zastavený'}")
        
        # 7. Uloženie snímku
        visualizer.save_frame("simulation_snapshot.png")
        
        # 8. Zatvorenie
        visualizer.close()
        engine.shutdown()
    
    print("\n✅ Simulácia dokončená!")

if __name__ == "__main__":
    main()
