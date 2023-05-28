#include "../livegraph_driver.hpp"
#include "../configuration.hpp"
#include "algorithms_experiment.hpp"
#include "updates_experiment.hpp"

class MixedV2Experiment {

public:
    MixedV2Experiment(LiveGraphDriver& driver, UpdateStream& update_stream) 
    : driver(driver), update_stream(update_stream), experiment_a(driver), experiment_u(driver, update_stream, false /*Don't use batch loader*/) {
        LOG("Starting Mixedv2 Experiment");
    }
    ~MixedV2Experiment() {};

    void execute();

    vector<int64_t> get_times();

private:
    LiveGraphDriver& driver;
    UpdateStream& update_stream;
    AlgorithmsExperiment experiment_a;
    UpdatesExperiment experiment_u;

};