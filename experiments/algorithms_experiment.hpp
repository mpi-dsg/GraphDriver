#ifndef ALGORITHMS_EXPERIMENT
#define ALGORITHMS_EXPERIMENT

#include <algorithm>
#include <cmath>

#include "../livegraph_driver.hpp"
#include "../configuration.hpp"

struct Statistics {
    int64_t min;
    int64_t max;
    int64_t sum;
    double average;
    double median;
    int64_t percentile90;
    int64_t percentile99;
    int64_t count;
};


class AlgorithmsExperiment{
public:
    AlgorithmsExperiment(LiveGraphDriver& driver) : driver(driver) {
        LOG("Starting Algorithms Experiment");
    }
    ~AlgorithmsExperiment() {};
    void execute(int repetitions = -1);
    vector<int64_t> get_times();
    static Statistics calculate_statistics(const vector<int64_t>& numbers);
private:
    LiveGraphDriver& driver;
    vector<int64_t> times;
};

#endif