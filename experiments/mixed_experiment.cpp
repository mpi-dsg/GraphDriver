#include <omp.h>

#include "mixed_experiment.hpp"


void MixedExperiment::execute() {
    // omp_set_num_threads(configuration().get_n_threads() + 1); // 1 for algorithms + others for updates
    bool terminate = false;
    omp_set_nested(1);
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            experiment_u.execute();

            #pragma omp atomic write
            terminate = true;
        }

        #pragma omp section
        {
            // sleep(10);
            while(!terminate) { // Keep on running algorithms until updates finish
                experiment_a.execute();
            }
        }
    }

    LOG("Total updates applied" << driver.updates_applied);
}

vector<int64_t> MixedExperiment::get_times() {
    return experiment_a.get_times();
}