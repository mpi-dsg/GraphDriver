#include <iostream>
#include <chrono>
#include <cxxopts.hpp>

#include "configuration.hpp"
// #include "livegraph_driver.hpp"
#include "reader/log_reader.hpp"
#include "experiments/algorithms_experiment.hpp"
#include "experiments/updates_experiment.hpp"
#include "experiments/mixed_experiment.hpp"
#include "experiments/sequential_experiment.hpp"

using namespace std;
using namespace std::chrono;



int main(int argc, char* argv[]){
    cxxopts::Options options("config");
    options.add_options()
        ("w,writer_threads", "Number of writer threads", cxxopts::value<int>()->default_value("1"))
        ("g,graph_path", "Path to graph properties", cxxopts::value<string>())
        ("v,validate", "Validate graph load and update", cxxopts::value<bool>()->default_value("false"))
        ("l,log_path", "Path to graph update log", cxxopts::value<string>()->default_value(""))
        ("b,batch_size", "Batch size for updates", cxxopts::value<uint64_t>()->default_value("1024"))
        ("R,repetitions", "Number of repetitions for algorithms", cxxopts::value<int>()->default_value("0"))
        ("r,rate", "Rate of updates to be applied for sequential", cxxopts::value<uint64_t>()->default_value("100000"))
        ("t,type", "Type of experiment(0 - insert only, 1 - algorithms only, 2 - updates only, 3 - mixed, 4 - sequential)", cxxopts::value<int>()->default_value("0"))
        ("h,help", "Print usage")
    ;
    auto result = options.parse(argc, argv);
    if(result.count("help")) {
        LOG(options.help());
        exit(0);
    }
    if(result.count("graph_path") == 0) {
        LOG("Graph path not provided");
        exit(1);
    }

    configuration().set_n_threads(result["writer_threads"].as<int>());
    configuration().set_batch_size(result["batch_size"].as<uint64_t>());
    configuration().set_repetitions(result["repetitions"].as<int>());
    configuration().set_rate(result["rate"].as<uint64_t>());
    int type = result["type"].as<int>();

    /*
        Loading EdgeStream
    */
    auto graph_path = result["graph_path"].as<string>();
    auto start = high_resolution_clock::now();
    auto stream = new EdgeStream(graph_path);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Stream loading time (in ms): " << duration.count());

    /*
        Loading Graph
    */
    auto driver = new LiveGraphDriver();
    bool validate = result["validate"].as<bool>();
    driver->load_graph(stream, configuration().get_n_threads(), validate);
    if(type == 0) exit(0);
    if(type == 1) {
        AlgorithmsExperiment experiment_a {driver};
        experiment_a.execute();
        exit(0);
    }

    /*
        Loading UpdateStream
    */
    string log_path = result["log_path"].as<string>();
    start = high_resolution_clock::now();
    auto update_stream = new UpdateStream(log_path);
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start);
    LOG("Update Stream loading time (in ms): " << duration.count());

    switch(type) {
        case 2:
        {
            UpdatesExperiment experiment_u {driver, update_stream};
            experiment_u.execute();
            break;
        }
        case 3:
        {
            MixedExperiment experiment_m {driver, update_stream};
            experiment_m.execute();
            break;
        }
        case 4:
        {
            SequentialExperiment experiment_s {driver, update_stream};
            experiment_s.execute();
            break;
        }
        default:
            LOG("Invalid Type: " << type);
            break;
    }
}