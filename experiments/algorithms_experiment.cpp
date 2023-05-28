#include <chrono>

#include "algorithms_experiment.hpp"

using namespace std::chrono;

void AlgorithmsExperiment::execute(int repetitions) {
    if(repetitions < 0) repetitions = configuration().get_repetitions();
    for(int i = 0; i < repetitions; i++){
        auto start = high_resolution_clock::now();
        auto output1 = driver.execute_bfs(0);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        auto time = duration.count();
        times.push_back(time);
        LOG("BFS Execution Time: " << time);

        // start = high_resolution_clock::now();
        // auto output2 = driver.execute_tc();
        // end = high_resolution_clock::now();
        // duration = duration_cast<milliseconds>(end - start);
        // time = duration.count();
        // times.push_back(time);
        // LOG("TC Execution Time: " << time);
    }
}

vector<int64_t> AlgorithmsExperiment::get_times() {
    return times;
}

Statistics AlgorithmsExperiment::calculate_statistics(const vector<int64_t>& numbers) {
    Statistics stats;
    if (numbers.empty()) {
        stats.min = 0;
        stats.max = 0;
        stats.sum = 0;
        stats.average = 0.0;
        stats.median = 0.0;
        stats.percentile90 = 0;
        stats.percentile99 = 0;
        stats.count = 0;
        return stats;
    }

    stats.min = *min_element(numbers.begin(), numbers.end());
    stats.max = *max_element(numbers.begin(), numbers.end());
    stats.sum = 0;
    for (int64_t num : numbers) {
        stats.sum += num;
    }
    stats.average = static_cast<double>(stats.sum) / numbers.size();

    // Calculate percentiles
    vector<int64_t> sortedNumbers = numbers;
    sort(sortedNumbers.begin(), sortedNumbers.end());

    auto size = numbers.size();
    stats.median = size % 2 == 0 ? (sortedNumbers[size / 2 - 1] + sortedNumbers[size / 2]) / 2.0 : sortedNumbers[size / 2];

    int index90 = static_cast<int>(round(0.9 * (sortedNumbers.size() - 1)));
    stats.percentile90 = sortedNumbers[index90];

    int index99 = static_cast<int>(round(0.99 * (sortedNumbers.size() - 1)));
    stats.percentile99 = sortedNumbers[index99];

    // Calculate count
    stats.count = numbers.size();

    return stats;
}