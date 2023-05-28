#include <omp.h>
#include <thread>

#include "sequentialv2_experiment.hpp"

using namespace std::chrono;

void SequentialV2Experiment::execute() {
    omp_set_nested(1);

    int n_threads = configuration().get_n_threads();
    uint64_t batch_size = configuration().get_batch_size();

    uint64_t size = update_stream.get_size();
    uint64_t num_batches = size/batch_size + (size % batch_size != 0);
    LOG("Size: " << size);
    LOG("Num batches:" << num_batches);
    auto graph = driver.get_graph();
    uint64_t done = 0;
    auto updates = update_stream.get_updates();
    for(uint64_t b_no = 0; b_no < num_batches; b_no++){
        LOG("Batch: " << b_no);
        uint64_t start = done;
        uint64_t end = min(done + batch_size, size);
        LOG("Just before");
        driver.update_graph_batch_part(updates, start, end, n_threads);
        done += batch_size;

        experiment_a.execute();

        // if(b_no != 0 && b_no % 50 == 0) {
        //     LOG("Compact");
        //     graph->compact();
        // }
    }

    LOG("Total Updates Applied: " << driver.updates_applied);
    
}

vector<int64_t> SequentialV2Experiment::get_times() {
    return experiment_a.get_times();
}