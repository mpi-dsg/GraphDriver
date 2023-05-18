#include "../livegraph_driver.hpp"
#include "../configuration.hpp"
#include "algorithms_experiment.hpp"
#include "updates_experiment.hpp"

class MixedExperiment {

public:
    MixedExperiment(LiveGraphDriver& driver, UpdateStream& update_stream) 
    : driver(driver), update_stream(update_stream), experiment_a(driver), experiment_u(driver, update_stream) {
        LOG("Starting Mixed Experiment");
    }
    ~MixedExperiment() {};

    void execute();

    vector<int64_t> get_times();

private:
    LiveGraphDriver& driver;
    UpdateStream& update_stream;
    AlgorithmsExperiment experiment_a;
    UpdatesExperiment experiment_u;

};