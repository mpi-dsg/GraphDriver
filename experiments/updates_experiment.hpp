#ifndef UPDATES_EXPERIMENT
#define UPDATES_EXPERIMENT

#include "../livegraph_driver.hpp"
#include "../configuration.hpp"

class UpdatesExperiment {
public:
    UpdatesExperiment(LiveGraphDriver& driver, UpdateStream& update_stream, bool use_batch_loader)
    : driver(driver), update_stream(update_stream), use_batch_loader(use_batch_loader) {
        LOG("Starting Updates Experiment");
    }
    ~UpdatesExperiment() {};

    void execute();

private:
    LiveGraphDriver& driver;
    UpdateStream& update_stream;
    bool use_batch_loader;
};

#endif