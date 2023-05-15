#include <chrono>

#include "mixed_experiment.hpp"
#include "algorithms_experiment.hpp"
#include "updates_experiment.hpp"

using namespace std::chrono;

void MixedExperiment::execute() {
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                UpdatesExperiment experiment {driver, update_stream};
                experiment.execute();
            }

            #pragma omp section
            {
                
                AlgorithmsExperiment experiment {driver};
                experiment.execute();
            }
        }
    }

    
}