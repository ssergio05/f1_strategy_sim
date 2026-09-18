import pandas as pd
import matplotlib.pyplot as plt
import os
import subprocess
import time
import json

def generate_mega_dashboard():
    plt.style.use('dark_background')
    plt.ion() 
    
    # Render 2 main panels to maximize the visual space for the race battle
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(22, 7))

    # Official constructor/driver branding colors
    driver_colors = {
        'Verstappen': '#1E41FF', # Red Bull Blue
        'Norris': '#FF8000',     # McLaren Papaya
        'Antonelli': '#00A19C',  # Mercedes/Petronas Green
        'Leclerc': '#FFD700',    # Ferrari Yellow/Gold (Alternative)
        'Hamilton': '#FF0000'    # Ferrari Red
    }

    while True:
        # Trigger the C++ engine to compute the latest Monte Carlo distributions
        subprocess.run(["build\\Release\\f1_simulator.exe"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        if not (os.path.exists('lap_times.csv') and os.path.exists('monte_carlo_results.csv')):
            time.sleep(2)
            continue
            
        try:
            df_laps = pd.read_csv('lap_times.csv')
            df_mc = pd.read_csv('monte_carlo_results.csv')
            
            with open('live_state.json', 'r') as f:
                live_data = json.load(f)
                active_drivers = [c['name'] for c in live_data.get('competitors', [])]
        except Exception:
            time.sleep(1)
            continue

        ax1.clear()
        ax2.clear()
        fig.suptitle(f'F1 AWS Live Strategy - Battle for the Win ({len(active_drivers)} Drivers)', fontsize=20, fontweight='bold', color='white')

        # --- PANEL 1: TELEMETRY & PIT STOPS ---
        competitor_cols = [col for col in df_laps.columns if col != 'Lap']
        for i, col in enumerate(competitor_cols):
            name = active_drivers[i] if i < len(active_drivers) else f"Car {i}"
            color = driver_colors.get(name, '#FFFFFF')
            ax1.plot(df_laps['Lap'], df_laps[col], color=color, linewidth=2.5, label=name)
        
        ax1.set_title('1. Race Telemetry (Degradation & Pits)', fontsize=14)
        ax1.set_xlabel('Lap Number', fontsize=12)
        ax1.set_ylabel('Lap Time (Seconds)', fontsize=12)
        ax1.grid(True, linestyle='--', alpha=0.3)
        if not df_laps.empty: ax1.legend()

        # --- PANEL 2: MONTE CARLO PROBABILITY DISTRIBUTION ---
        mc_cols = [col for col in df_mc.columns if col != 'SimulationID']
        for i, col in enumerate(mc_cols):
            name = active_drivers[i] if i < len(active_drivers) else f"Car {i}"
            color = driver_colors.get(name, '#FFFFFF')
            times = df_mc[col] / 60.0
            ax2.hist(times, bins=40, alpha=0.7, color=color, label=f'{name} ({times.mean():.2f}m)')
        
        ax2.set_title('2. Monte Carlo Total Race Times', fontsize=14)
        ax2.set_xlabel('Total Time (Minutes)', fontsize=12)
        ax2.grid(True, linestyle='--', alpha=0.3)
        if not df_mc.empty: ax2.legend()

        plt.tight_layout()
        plt.subplots_adjust(top=0.88)
        
        fig.canvas.draw_idle()
        fig.canvas.flush_events()
        
        # Idle loop before next engine trigger
        for _ in range(100):
            fig.canvas.flush_events()
            time.sleep(0.1)

if __name__ == "__main__":
    generate_mega_dashboard()