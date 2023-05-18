#include <chrono>
#include <omp.h>

#include "sequential_experiment.hpp"

using namespace std::chrono;

void SequentialExperiment::execute() {
    omp_set_nested(1);
    auto start = high_resolution_clock::now();
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            driver.start_updates(update_stream, 1);
        }

        #pragma omp section
        {
            LOG("Batch update with threads: " << configuration().get_n_threads());
            LOG("Batch update with batch_size: " << configuration().get_batch_size());
            sleep(25);
            while(!driver.stop_sequential()) {
                experiment_a.execute();
                driver.apply_updates();
            }
        }
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Sequential Experiment time (in ms): " << duration.count() << "\n");
    LOG("Total updates applied" << driver.updates_applied);
}

vector<int64_t> SequentialExperiment::get_times() {
    return experiment_a.get_times();
}