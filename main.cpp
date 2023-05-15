#include <iostream>
#include <chrono>
#include <cxxopts.hpp>

#include "configuration.hpp"
// #include "livegraph_driver.hpp"
#include "reader/log_reader.hpp"
#include "experiments/algorithms_experiment.hpp"
#include "experiments/updates_experiment.hpp"
#include "experiments/mixed_experiment.hpp"

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
        ("r,repetitions", "Number of repetitions for algorithms", cxxopts::value<int>()->default_value("0"))
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

    auto graph_path = result["graph_path"].as<string>();
    auto start = high_resolution_clock::now();
    auto stream = new EdgeStream(graph_path);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Stream loading time (in ms): " << duration.count());

    auto driver = new LiveGraphDriver();
    bool validate = result["validate"].as<bool>();
    driver->load_graph(stream, configuration().get_n_threads(), validate);
    // auto output = driver->execute_bfs(0);

    // string log_path = result.count("log_path") > 0 ? result["log_path"].as<string>() : "";
    string log_path = result["log_path"].as<string>();
    if(log_path.length() > 0){
        start = high_resolution_clock::now();
        auto update_stream = new UpdateStream(log_path);
        end = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(end - start);
        LOG("Update Stream loading time (in ms): " << duration.count());

        if(configuration().get_repetitions() > 0) { 
            MixedExperiment experiment {driver, update_stream};
            experiment.execute();
        }
        else {
            UpdatesExperiment experiment {driver, update_stream};
            experiment.execute();
        }
    }
    else{
        AlgorithmsExperiment experiment {driver};
        experiment.execute();
    }
}