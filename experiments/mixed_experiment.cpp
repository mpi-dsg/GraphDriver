#include <chrono>
#include <omp.h>

#include "mixed_experiment.hpp"
#include "algorithms_experiment.hpp"
#include "updates_experiment.hpp"

using namespace std::chrono;

void MixedExperiment::execute() {
    omp_set_num_threads(configuration().get_n_threads() + 1); // 1 for algorithms + others for updates
    #pragma omp parallel
    {
        int threadId = omp_get_thread_num();

        // Bind thread 0 (function1) to core 0
        if (threadId == 0) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(0, &cpuset);
            sched_setaffinity(0, sizeof(cpuset), &cpuset);
            
            AlgorithmsExperiment experiment {driver};
            experiment.execute();
        }

        #pragma omp single
        {
            UpdatesExperiment experiment {driver, update_stream};
            experiment.execute();
        }
    }
}