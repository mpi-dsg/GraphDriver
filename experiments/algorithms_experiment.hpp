#include "../livegraph_driver.hpp"
#include "../configuration.hpp"

class AlgorithmsExperiment{
public:
    AlgorithmsExperiment(LiveGraphDriver* driver) : driver(driver) {
        LOG("Starting Algorithms Experiment");
    }
    ~AlgorithmsExperiment() {};
    void execute();
private:
    LiveGraphDriver* driver;
};