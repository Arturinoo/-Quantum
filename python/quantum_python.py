# Vytvor priečinok ak neexistuje
New-Item -ItemType Directory -Force -Path python

# Vytvor súbor
@"
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

print("🚀 Quantum Engine - Python Only Version")
print("="*50)

class QuantumParticle:
    def __init__(self, grid_points=512, x_range=10):
        self.N = grid_points
        self.x = np.linspace(-x_range/2, x_range/2, self.N)
        self.dx = self.x[1] - self.x[0]
        self.psi = np.exp(-0.5 * ((self.x - 0) / 0.5)**2) * np.exp(1j * 2 * self.x)
        self.normalize()
        self.V = np.zeros_like(self.x)
        for i, x in enumerate(self.x):
            if -1.5 < x < -0.8 or 0.8 < x < 1.5:
                self.V[i] = 10.0
        self.dt = 0.001
        self.hbar = 1.0
        self.mass = 1.0
        
    def normalize(self):
        norm = np.sqrt(np.sum(np.abs(self.psi)**2) * self.dx)
        if norm > 0:
            self.psi /= norm
    
    def step(self):
        k = np.fft.fftfreq(self.N, self.dx) * 2 * np.pi
        self.psi = np.fft.fft(self.psi)
        self.psi *= np.exp(-0.5j * self.dt * self.hbar * k**2 / (2 * self.mass))
        self.psi = np.fft.ifft(self.psi)
        self.psi *= np.exp(-1j * self.dt * self.V / self.hbar)
        self.psi = np.fft.fft(self.psi)
        self.psi *= np.exp(-0.5j * self.dt * self.hbar * k**2 / (2 * self.mass))
        self.psi = np.fft.ifft(self.psi)
        self.normalize()

class ClassicalParticles:
    def __init__(self, n=800):
        self.n = n
        self.pos = np.random.randn(n, 2) * 3
        self.vel = np.random.randn(n, 2) * 0.05
        self.mass = np.ones(n) * 0.1
        self.G = 0.05
        self.dt = 0.005
        for i in range(n):
            r = np.random.uniform(0.3, 3)
            theta = np.random.uniform(0, 2*np.pi)
            self.pos[i, 0] = r * np.cos(theta)
            self.pos[i, 1] = r * np.sin(theta)
            v = 0.7 / np.sqrt(r + 0.05)
            self.vel[i, 0] = -v * np.sin(theta)
            self.vel[i, 1] = v * np.cos(theta)
    
    def step(self):
        diff = self.pos[:, None, :] - self.pos[None, :, :]
        dist2 = np.sum(diff**2, axis=2) + 0.01
        dist = np.sqrt(dist2)
        force = self.G * self.mass[None, :] / dist2
        acc = np.sum(diff * force[:, :, None] / dist[:, :, None], axis=1)
        self.vel += acc * self.dt
        self.pos += self.vel * self.dt
        mask = np.abs(self.pos) > 5
        self.pos[mask] *= 0.99

print("📦 Inicializácia simulácie...")
quantum = QuantumParticle(512, 10)
classical = ClassicalParticles(800)

fig = plt.figure(figsize=(14, 8))
fig.patch.set_facecolor('#0a0a0f')

# 4 subploty
ax1 = plt.subplot(2, 2, 1)
ax1.set_title('⚛️ Kvantová vlnová funkcia', color='white')
ax1.set_facecolor('black')
ax1.grid(True, alpha=0.2)

ax2 = plt.subplot(2, 2, 2)
ax2.set_title('🔮 Potenciál V(x)', color='white')
ax2.set_facecolor('black')
ax2.grid(True, alpha=0.2)

ax3 = plt.subplot(2, 2, 3)
ax3.set_title('🌌 Klasické častice', color='white')
ax3.set_facecolor('black')
ax3.grid(True, alpha=0.2)

ax4 = plt.subplot(2, 2, 4)
ax4.set_title('🔗 Energia systému', color='white')
ax4.set_facecolor('black')
ax4.grid(True, alpha=0.2)

time = 0
energy_history = []
time_history = []
step_count = 0

def update(frame):
    global time, step_count
    for _ in range(10):
        quantum.step()
        classical.step()
        time += quantum.dt * 10
        step_count += 1
    
    # Kvantová vlna
    ax1.clear()
    prob = np.abs(quantum.psi)**2
    ax1.fill_between(quantum.x, 0, prob, alpha=0.5, color='cyan')
    ax1.plot(quantum.x, prob, 'c-', linewidth=2)
    ax1.set_xlim(-5, 5)
    ax1.set_ylim(0, max(prob) * 1.2 if max(prob) > 0.001 else 0.5)
    ax1.set_title(f'⚛️ Kvantová vlna | t={time:.2f}s', color='white')
    ax1.set_facecolor('black')
    ax1.grid(True, alpha=0.2)
    for spine in ax1.spines.values():
        spine.set_color('white')
    ax1.tick_params(colors='white')
    
    # Potenciál
    ax2.clear()
    ax2.fill_between(quantum.x, 0, quantum.V, alpha=0.3, color='red')
    ax2.plot(quantum.x, quantum.V, 'r-', linewidth=2)
    ax2.set_xlim(-5, 5)
    ax2.set_title('🔮 Potenciál V(x)', color='white')
    ax2.set_facecolor('black')
    ax2.grid(True, alpha=0.2)
    for spine in ax2.spines.values():
        spine.set_color('white')
    ax2.tick_params(colors='white')
    
    # Klasické častice
    ax3.clear()
    idx = np.random.choice(classical.n, min(2000, classical.n), replace=False)
    ax3.scatter(classical.pos[idx, 0], classical.pos[idx, 1], 
               c='lime', s=2, alpha=0.6)
    ax3.set_xlim(-5, 5)
    ax3.set_ylim(-5, 5)
    ax3.set_title(f'🌌 Častíc: {classical.n} | Krok: {step_count}', color='white')
    ax3.set_facecolor('black')
    ax3.grid(True, alpha=0.2)
    for spine in ax3.spines.values():
        spine.set_color('white')
    ax3.tick_params(colors='white')
    
    # Energia
    energy = np.sum(np.abs(quantum.psi)**2) + classical.n * 0.1
    energy_history.append(energy)
    time_history.append(time)
    if len(energy_history) > 300:
        energy_history.pop(0)
        time_history.pop(0)
    
    ax4.clear()
    ax4.plot(time_history, energy_history, 'y-', linewidth=2, alpha=0.8)
    ax4.set_xlim(max(0, time - 3), max(time, 1))
    ax4.set_ylim(min(energy_history) * 0.9 if energy_history else 0, 
                 max(energy_history) * 1.1 if energy_history else 100)
    ax4.set_title('🔗 Energia systému', color='white')
    ax4.set_facecolor('black')
    ax4.grid(True, alpha=0.2)
    for spine in ax4.spines.values():
        spine.set_color('white')
    ax4.tick_params(colors='white')
    
    plt.tight_layout()
    
    if step_count % 100 == 0:
        print(f"⏱️  Krok: {step_count} | Čas: {time:.2f}s | Energia: {energy:.3f}")
    
    return ax1, ax2, ax3, ax4

print("🎬 Spúšťam animáciu...")
print("   (Zatvor okno pre ukončenie)\n")

ani = FuncAnimation(fig, update, interval=50, cache_frame_data=False)
plt.show()

print(f"\n✅ Simulácia dokončená!")
print(f"   Celkový čas: {time:.2f}s")
print(f"   Počet krokov: {step_count}")
"@ | Out-File -FilePath python\quantum_python.py -Encoding UTF8