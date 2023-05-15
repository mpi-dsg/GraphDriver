#include "../livegraph_driver.hpp"
#include "../configuration.hpp"

class MixedExperiment {

public:
    MixedExperiment(LiveGraphDriver* driver, UpdateStream* update_stream) 
    : driver(driver), update_stream(update_stream) {
        LOG("Starting Mixed Experiment");
    }
    ~MixedExperiment() {};

    void execute();

private:
    LiveGraphDriver* driver;
    UpdateStream* update_stream;

};