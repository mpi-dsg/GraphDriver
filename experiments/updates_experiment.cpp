#include <chrono>

#include "updates_experiment.hpp"

using namespace std::chrono;

void UpdatesExperiment::execute() {
    if(use_batch_loader) {
        LOG("Using Batch Loader");
        driver.update_graph_batch(update_stream, configuration().get_batch_size(), configuration().get_n_threads());
    }
    else {
        LOG("Not using Batch Loader");
        driver.update_graph(update_stream, configuration().get_n_threads());
    }
}