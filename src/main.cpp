#include <iostream>
#include <memory>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <optional>
#include <algorithm>
#include <utility>

#include <nlohmann/json.hpp>

#include "f1sim/TyreModel.hpp"
#include "f1sim/Car.hpp"
#include "f1sim/Strategy.hpp"
#include "f1sim/RaceSession.hpp"
#include "f1sim/MonteCarloEngine.hpp"
#include "f1sim/CSVExporter.hpp"

using namespace f1sim;
using json = nlohmann::json;

namespace {

constexpr double BASE_LAP_TIME = 90.0;
constexpr double FULL_FUEL_MASS = 110.0;
constexpr double FUEL_DELTA = 0.03;
constexpr uint32_t NUM_SIMULATIONS = 10000;

struct LiveState {
    int currentLap{1};
    int totalLaps{70};
    bool safetyCar{false};
    std::vector<Competitor> grid;
};

std::shared_ptr<TyreModel> makeTyreForCompound(const std::string& compound) {
    if (compound == "Soft")   return std::make_shared<TyreModel>(0.10, 0.05, 0.20, 15.0);
    if (compound == "Medium") return std::make_shared<TyreModel>(0.06, 0.03, 0.15, 25.0);
    if (compound == "Hard")   return std::make_shared<TyreModel>(0.04, 0.02, 0.10, 40.0);
    return nullptr;
}

std::optional<LiveState> loadLiveStateFromJson(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return std::nullopt;

    json root;
    try { file >> root; } catch (...) { return std::nullopt; }

    LiveState state;
    try {
        state.currentLap = root.at("session").at("current_lap").get<int>();
        state.totalLaps  = root.at("session").at("total_laps").get<int>();
        state.safetyCar  = root.at("session").at("safety_car").get<bool>();

        std::vector<std::pair<int, Competitor>> ordered;
        for (const auto& entry : root.at("competitors")) {
            const std::string name = entry.at("name").get<std::string>();
            const int position = entry.at("position").get<int>();
            const std::string compound = entry.at("current_tyre").get<std::string>();
            const double lapsOnTyre = entry.at("laps_on_tyre").get<double>();

            auto tyre = makeTyreForCompound(compound);
            if (!tyre) continue;

            Car car(BASE_LAP_TIME, FULL_FUEL_MASS, FUEL_DELTA, tyre);
            const double currentFuel = FULL_FUEL_MASS - (state.currentLap * FUEL_DELTA);
            car.setMidRaceState(currentFuel, lapsOnTyre);

            // DYNAMIC TYRE LIFE CALCULATION
            double maxLife = 20.0; // Default for Soft
            if (compound == "Medium") maxLife = 35.0;
            if (compound == "Hard") maxLife = 55.0;

            int pitLap = state.currentLap + (static_cast<int>(maxLife) - static_cast<int>(lapsOnTyre));
            
            if (pitLap <= state.currentLap) pitLap = state.currentLap + 2; 
            if (pitLap >= state.totalLaps) pitLap = 0; // Attempt to finish without pitting

            auto nextTyre = (compound == "Hard") ? makeTyreForCompound("Medium") : makeTyreForCompound("Hard");
            Strategy strategy{pitLap, tyre, nextTyre};

            ordered.emplace_back(position, Competitor{name, std::move(car), std::move(strategy)});
        }
        std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
        for (auto& [pos, comp] : ordered) state.grid.push_back(std::move(comp));
    } catch (...) { return std::nullopt; }
    return state;
}

} // namespace

int main(int argc, char* argv[]) {
    std::cout << "--- F1 GRAND STRATEGY ENGINE (Live Predictor) ---\n\n";

    std::string liveStatePath = "live_state.json";
    if (argc > 1) liveStatePath = argv[1];

    auto liveState = loadLiveStateFromJson(liveStatePath);
    if (!liveState) {
        std::cerr << "[Live Predictor] ERROR: Failed to parse JSON telemetry.\n";
        return 1;
    }

    // ==========================================
    // PHASE 1: LIVE RACE PREDICTION
    // ==========================================
    std::cout << "-> Simulating remaining race from lap " << liveState->currentLap << "...\n";
    RaceSession raceSession(liveState->grid, liveState->totalLaps, liveState->currentLap);
    raceSession.setInitialSafetyCar(liveState->safetyCar);

    MonteCarloEngine engine;
    auto resultsMC = engine.runSimulations(NUM_SIMULATIONS, raceSession);

    CSVExporter::exportLapTimes(resultsMC[0], "lap_times.csv", liveState->currentLap);
    CSVExporter::exportMonteCarloResults(resultsMC, "monte_carlo_results.csv");

    // ==========================================
    // PHASE 2: LIVE STRATEGY OPTIMIZER
    // ==========================================
    std::cout << "-> Recalculating Strategy Optimizer from current stint...\n";
    std::ofstream optFile("optimization_results.csv");
    optFile << "PitLap,SoftHard,MedHard,MedSoft\n";

    auto softTyre = makeTyreForCompound("Soft");
    auto mediumTyre = makeTyreForCompound("Medium");
    auto hardTyre = makeTyreForCompound("Hard");

    // Sweep from current lap onwards
    int startOptLap = liveState->currentLap + 1;
    for (int pitLap = startOptLap; pitLap <= liveState->totalLaps - 2; ++pitLap) {
        std::vector<Competitor> optGrid;
        
        Car carSH(BASE_LAP_TIME, FULL_FUEL_MASS, FUEL_DELTA, softTyre);
        carSH.setMidRaceState(FULL_FUEL_MASS - (liveState->currentLap * FUEL_DELTA), 0); 
        optGrid.push_back({"OptSH", carSH, Strategy{pitLap, softTyre, hardTyre}});

        Car carMH(BASE_LAP_TIME, FULL_FUEL_MASS, FUEL_DELTA, mediumTyre);
        carMH.setMidRaceState(FULL_FUEL_MASS - (liveState->currentLap * FUEL_DELTA), 0);
        optGrid.push_back({"OptMH", carMH, Strategy{pitLap, mediumTyre, hardTyre}});

        Car carMS(BASE_LAP_TIME, FULL_FUEL_MASS, FUEL_DELTA, mediumTyre);
        carMS.setMidRaceState(FULL_FUEL_MASS - (liveState->currentLap * FUEL_DELTA), 0);
        optGrid.push_back({"OptMS", carMS, Strategy{pitLap, mediumTyre, softTyre}});

        RaceSession optSession(optGrid, liveState->totalLaps, liveState->currentLap);
        auto optResults = engine.runSimulations(500, optSession);

        double tSH = 0, tMH = 0, tMS = 0;
        for (const auto& r : optResults) {
            tSH += r.competitorResults[0].totalRaceTime;
            tMH += r.competitorResults[1].totalRaceTime;
            tMS += r.competitorResults[2].totalRaceTime;
        }
        optFile << pitLap << "," << (tSH/500) << "," << (tMH/500) << "," << (tMS/500) << "\n";
    }

    std::cout << "\n--- DONE. Live prediction completed successfully. ---\n";
    return 0;
}