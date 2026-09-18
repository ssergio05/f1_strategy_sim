#include "f1sim/MonteCarloEngine.hpp"

#include <algorithm>
#include <future>
#include <thread>
#include <vector>

namespace f1sim {

std::vector<SimulationResult> MonteCarloEngine::runSimulations(
    uint32_t numSimulations, const RaceSession& baseSession)
{
    uint32_t numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) {
        numThreads = 2; // Fallback for systems that cannot detect hardware concurrency
    }

    uint32_t chunkSize = (numSimulations + numThreads - 1) / numThreads;

    std::vector<std::future<std::vector<SimulationResult>>> futures;
    futures.reserve(numThreads);

    uint32_t start = 0;
    for (uint32_t t = 0; t < numThreads && start < numSimulations; ++t) {
        uint32_t end = std::min(start + chunkSize, numSimulations);
        
        // Dispatch parallel simulation workloads
        futures.push_back(std::async(std::launch::async, [&baseSession, start, end]() {
            std::vector<SimulationResult> chunkResults;
            chunkResults.reserve(end - start);
            for (uint32_t i = start; i < end; ++i) {
                RaceSession sessionCopy = baseSession;
                chunkResults.push_back(sessionCopy.run());
            }
            return chunkResults;
        }));
        start = end;
    }

    // Aggregate results from all threads
    std::vector<SimulationResult> allResults;
    allResults.reserve(numSimulations);
    for (auto& future : futures) {
        auto chunk = future.get();
        allResults.insert(allResults.end(),
            std::make_move_iterator(chunk.begin()),
            std::make_move_iterator(chunk.end()));
    }

    return allResults;
}

} // namespace f1sim