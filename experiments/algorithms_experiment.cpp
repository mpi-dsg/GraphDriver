#include <chrono>

#include "algorithms_experiment.hpp"

using namespace std::chrono;

void AlgorithmsExperiment::execute() {
    vector<int64_t> times;
    for(int i = 0; i < configuration().get_repetitions(); i++){
        auto start = high_resolution_clock::now();
        auto output = driver->execute_bfs(0);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        auto time = duration.count();
        times.push_back(time);
        LOG(time);
    }

    for(auto time: times) LOG(time);
}
