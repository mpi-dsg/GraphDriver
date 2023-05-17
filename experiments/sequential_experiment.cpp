#include <chrono>
#include <omp.h>

#include "sequential_experiment.hpp"

using namespace std::chrono;

void SequentialExperiment::execute() {
    // omp_set_num_threads(configuration().get_n_threads() + 1);
    // omp_set_nested(1);
    // #pragma omp parallel
    // {
    //     int threadId = omp_get_thread_num();
    //     if (threadId == 0) {
    //         cpu_set_t cpuset;
    //         CPU_ZERO(&cpuset);
    //         CPU_SET(0, &cpuset);
    //         sched_setaffinity(0, sizeof(cpuset), &cpuset);
            
    //         AlgorithmsExperiment experiment {driver};
    //         while(!driver->stop_sequential()) {
    //             experiment.execute();
    //             // driver->apply_updates();
    //         }
    //     }
    //     #pragma omp single
    //     {
    //         driver->start_updates(update_stream, configuration().get_n_threads());
    //     }
    // }
    //         AlgorithmsExperiment experiment {driver};
    //         experiment.execute();

    omp_set_nested(1);
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            driver->start_updates(update_stream, 1);
        }

        #pragma omp section
        {
            while(!driver->stop_sequential()) {
                experiment_a.execute();
                driver->apply_updates();
            }
            
        }
    }
}

vector<int64_t> SequentialExperiment::get_times() {
    return experiment_a.get_times();
}