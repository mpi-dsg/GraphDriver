#ifndef ALGORITHMS_EXPERIMENT
#define ALGORITHMS_EXPERIMENT

#include "../livegraph_driver.hpp"
#include "../configuration.hpp"

class AlgorithmsExperiment{
public:
    AlgorithmsExperiment(LiveGraphDriver* driver) : driver(driver) {
        LOG("Starting Algorithms Experiment");
    }
    ~AlgorithmsExperiment() {};
    void execute();
    vector<int64_t> get_times();
private:
    LiveGraphDriver* driver;
    vector<int64_t> times;
};

#endif