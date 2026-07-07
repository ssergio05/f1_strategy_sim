import os
import pandas as pd
import matplotlib.pyplot as plt

# Paths relative to the root directory where the script will be executed
BUILD_DIR = "build"
LAP_TIMES_FILE = os.path.join(BUILD_DIR, "lap_times.csv")
MC_RESULTS_FILE = os.path.join(BUILD_DIR, "monte_carlo_results.csv")

def generate_f1_dashboard():
    # Setup professional dark F1 telemetry style
    plt.style.use('dark_background')
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))
    fig.suptitle('F1 Strategy & Telemetry Dashboard', fontsize=16, fontweight='bold')

    # --- Plot 1: Lap Times & Degradation Curve ---
    if os.path.exists(LAP_TIMES_FILE):
        df_laps = pd.read_csv(LAP_TIMES_FILE)
        ax1.plot(df_laps['Lap'], df_laps['LapTime'], color='cyan', linewidth=2, label='Lap Time')
        
        # Highlight the pitstop (max time)
        pit_lap_idx = df_laps['LapTime'].idxmax()
        pit_lap = df_laps.iloc[pit_lap_idx]['Lap']
        ax1.scatter(pit_lap, df_laps.iloc[pit_lap_idx]['LapTime'], color='red', s=100, zorder=5, label=f'Pitstop (Lap {int(pit_lap)})')
        
        ax1.set_title("Single Race Degradation Profile", color='lightgray')
        ax1.set_xlabel("Lap Number")
        ax1.set_ylabel("Lap Time (Seconds)")
        ax1.grid(color='gray', linestyle='--', linewidth=0.5, alpha=0.5)
        ax1.legend()
    else:
        ax1.text(0.5, 0.5, 'lap_times.csv missing', ha='center', color='red')

    # --- Plot 2: Monte Carlo Histogram ---
    if os.path.exists(MC_RESULTS_FILE):
        df_mc = pd.read_csv(MC_RESULTS_FILE)
        # Convert total seconds to minutes for better readability
        df_mc['TotalRaceTime_Min'] = df_mc['TotalRaceTime'] / 60.0
        
        # Plot histogram
        ax2.hist(df_mc['TotalRaceTime_Min'], bins=50, color='magenta', edgecolor='black', alpha=0.8)
        
        # Add Mean and standard deviation vertical lines
        mean_time = df_mc['TotalRaceTime_Min'].mean()
        ax2.axvline(mean_time, color='yellow', linestyle='dashed', linewidth=2, label=f'Mean: {mean_time:.2f} min')
        
        ax2.set_title(f"Monte Carlo Distribution ({len(df_mc)} Sims)", color='lightgray')
        ax2.set_xlabel("Total Race Time (Minutes)")
        ax2.set_ylabel("Frequency")
        ax2.grid(color='gray', linestyle='--', linewidth=0.5, alpha=0.5)
        ax2.legend()
    else:
        ax2.text(0.5, 0.5, 'monte_carlo_results.csv missing', ha='center', color='red')

    # Finalize and show
    plt.tight_layout()
    # Save the output as a high-res image
    plt.savefig("f1_strategy_dashboard.png", dpi=300, bbox_inches='tight')
    print("Dashboard generated successfully: f1_strategy_dashboard.png")
    plt.show()

if __name__ == "__main__":
    generate_f1_dashboard()