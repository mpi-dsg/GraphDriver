#include <omp.h>
#include <thread>
#include <sched.h>

#include "mixedv2_experiment.hpp"

using namespace std;

static void update_function(UpdatesExperiment& experiment_u) {
    LOG("T1");
    experiment_u.execute();
}

static void algorithm_function(UpdateStream& update_stream, AlgorithmsExperiment& experiment_a) {
    LOG("T2");
    uint64_t num_updates = update_stream.get_size();
    uint64_t batch_size = configuration().get_batch_size();
    int reps = configuration().get_repetitions();
    int repetitions = ( num_updates/batch_size + (num_updates % batch_size!=0) ) * reps;
    experiment_a.execute(repetitions);
}

void MixedV2Experiment::execute() {
    // omp_set_num_threads(configuration().get_n_threads() + 1); // 1 for algorithms + others for updates
    omp_set_nested(1);
    LOG("Threads Version");
    cpu_set_t cpuset1;
    cpu_set_t cpuset2;
    CPU_ZERO(&cpuset1);
    CPU_ZERO(&cpuset2);

    int cores = configuration().get_cores(); // Update cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    for(int i = 0; i < cores; i++) CPU_SET(i, &cpuset1);
    for(int i = cores; i < num_cores; i++) CPU_SET(i, &cpuset2);

    LOG("Total cores: " << num_cores);
    LOG("Update cores: " << cores);

    // Update thread
    thread t1(update_function, ref(experiment_u));
    int res = pthread_setaffinity_np(t1.native_handle(), sizeof(cpuset1), &cpuset1);
    LOG("T1 affinity set: " << res);
    res = pthread_getaffinity_np(t1.native_handle(), sizeof(cpuset1), &cpuset1);
    LOG("T1 affinity get: " << res);
    string affinity_str = "";
    for (int i = 0; i < num_cores; ++i) {
        if(CPU_ISSET(i, &cpuset1)) affinity_str += "1 ";
        else affinity_str += "0 ";
        
    }
    LOG(affinity_str);

    // Algorithm thread
    thread t2(algorithm_function, ref(update_stream), ref(experiment_a));
    affinity_str = "";
    res = pthread_setaffinity_np(t2.native_handle(), sizeof(cpuset2), &cpuset2);
    LOG("T2 affinity set: " << res);
    res = pthread_getaffinity_np(t2.native_handle(), sizeof(cpuset2), &cpuset2);
    LOG("T2 affinity get: " << res);
    for (int i = 0; i < num_cores; ++i) {
        if(CPU_ISSET(i, &cpuset2)) affinity_str += "1 ";
        else affinity_str += "0 ";
    }
    LOG(affinity_str);

    t1.join();
    t2.join();

    LOG("Total updates applied: " << driver.updates_applied);
}

vector<int64_t> MixedV2Experiment::get_times() {
    return experiment_a.get_times();
}