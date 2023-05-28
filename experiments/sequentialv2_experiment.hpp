#ifndef SEQUENTIALV2_EXPERIMENT
#define SEQUENTIALV2_EXPERIMENT

#include "../livegraph_driver.hpp"
#include "../configuration.hpp"
#include "algorithms_experiment.hpp"

class SequentialV2Experiment {

public:
    SequentialV2Experiment(LiveGraphDriver& driver, UpdateStream& update_stream) 
    : driver(driver), update_stream(update_stream), experiment_a(driver) {
        LOG("Starting Sequentialv2 Experiment");
    }
    ~SequentialV2Experiment() {};

    void execute();

    vector<int64_t> get_times();

private:
    LiveGraphDriver& driver;
    UpdateStream& update_stream;
    AlgorithmsExperiment experiment_a;
};

#endif