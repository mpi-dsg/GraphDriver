#ifndef SEQUENTIAL_EXPERIMENT
#define SEQUENTIAL_EXPERIMENT

#include "../livegraph_driver.hpp"
#include "../configuration.hpp"
#include "algorithms_experiment.hpp"

class SequentialExperiment {

public:
    SequentialExperiment(LiveGraphDriver& driver, UpdateStream& update_stream) 
    : driver(driver), update_stream(update_stream), experiment_a(driver) {
        LOG("Starting Sequential Experiment");
    }
    ~SequentialExperiment() {};

    void execute();

    vector<int64_t> get_times();

private:
    LiveGraphDriver& driver;
    UpdateStream& update_stream;
    AlgorithmsExperiment experiment_a;
};

#endif