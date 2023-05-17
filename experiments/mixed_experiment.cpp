#include <omp.h>

#include "mixed_experiment.hpp"


void MixedExperiment::execute() {
    // omp_set_num_threads(configuration().get_n_threads() + 1); // 1 for algorithms + others for updates
    omp_set_nested(1);

    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            experiment_u.execute();
            
        }

        #pragma omp section
        {
            experiment_a.execute();
        }
    }
}

vector<int64_t> MixedExperiment::get_times() {
    return experiment_a.get_times();
}