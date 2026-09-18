import pandas as pd
import matplotlib.pyplot as plt

def plot_optimization():
    # Load optimization data computed by the C++ Monte Carlo engine
    df = pd.read_csv('optimization_results.csv')
    df['AverageRaceTimeMin'] = df['AverageRaceTime'] / 60.0

    # Find the optimal pit stop lap (minimum total race time)
    optimal_idx = df['AverageRaceTimeMin'].idxmin()
    optimal_lap = df.loc[optimal_idx, 'PitLap']
    optimal_time = df.loc[optimal_idx, 'AverageRaceTimeMin']

    plt.style.use('dark_background')
    plt.figure(figsize=(10, 6))
    
    plt.plot(df['PitLap'], df['AverageRaceTimeMin'], color='cyan', marker='o', linewidth=2)
    plt.scatter(optimal_lap, optimal_time, color='red', s=150, zorder=5, 
                label=f'Optimal Lap: {int(optimal_lap)}\nTime: {optimal_time:.2f} min')
    
    plt.title('F1 Strategy Optimizer (Soft -> Hard)', fontsize=16, fontweight='bold')
    plt.xlabel('Pit Stop Lap', fontsize=12)
    plt.ylabel('Average Race Time (Minutes)', fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.3)
    plt.legend(fontsize=12)
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_optimization()