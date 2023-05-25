#include <chrono>
#include <omp.h>

#include "sequential_experiment.hpp"

using namespace std::chrono;

void SequentialExperiment::execute() {
    omp_set_nested(1);
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
            while(!driver.stop_sequential()) {
                driver.apply_updates();
                // driver.get_graph()->compact();
                experiment_a.execute();
            }
        }
    }
    LOG("Total updates applied: " << driver.updates_applied);
}

vector<int64_t> SequentialExperiment::get_times() {
    return experiment_a.get_times();
}