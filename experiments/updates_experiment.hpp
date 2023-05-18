#ifndef UPDATES_EXPERIMENT
#define UPDATES_EXPERIMENT

#include "../livegraph_driver.hpp"
#include "../configuration.hpp"

class UpdatesExperiment {
public:
    UpdatesExperiment(LiveGraphDriver& driver, UpdateStream& update_stream)
    : driver(driver), update_stream(update_stream) {
        LOG("Starting Updates Experiment");
    }
    ~UpdatesExperiment() {};

    void execute();

private:
    LiveGraphDriver& driver;
    UpdateStream& update_stream;
};

#endif