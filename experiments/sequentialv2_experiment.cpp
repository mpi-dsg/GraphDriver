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
        experiment_a.execute();
        
        LOG("Batch: " << b_no);
        uint64_t start = done;
        uint64_t end = min(done + batch_size, size);
        driver.update_graph_batch_part(updates, start, end, n_threads);
        done += batch_size;

        // auto time_start = high_resolution_clock::now();
        // graph->compact();
        // auto time_end = high_resolution_clock::now();
        // auto duration = duration_cast<milliseconds>(time_end - time_start);
        // LOG("Compaction time(in ms): " << duration.count());
    }

    LOG("Total Updates Applied: " << driver.updates_applied);
    
}

vector<int64_t> SequentialV2Experiment::get_times() {
    return experiment_a.get_times();
}