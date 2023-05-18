#include <chrono>

#include "updates_experiment.hpp"

using namespace std::chrono;

void UpdatesExperiment::execute() {
    auto start = high_resolution_clock::now();
    driver.update_graph_batch(update_stream, configuration().get_batch_size(), configuration().get_n_threads());
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Update application time (in ms): " << duration.count() << "\n");
}