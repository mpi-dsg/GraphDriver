#include <iostream>
#include <chrono>
#include <cxxopts.hpp>

#include "configuration.hpp"
#include "livegraph_driver.hpp"
#include "reader/log_reader.hpp"

using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[]){
    cxxopts::Options options("config");
    options.add_options()
        ("w,writer_threads", "Number of writer threads", cxxopts::value<int>()->default_value("1"))
        ("g,graph_path", "Path to graph properties", cxxopts::value<string>())
        ("v,validate", "Validate graph load and update", cxxopts::value<bool>()->default_value("false"))
        ("l,log_path", "Path to graph update log", cxxopts::value<string>())
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

    auto graph_path = result["graph_path"].as<string>();
    auto start = high_resolution_clock::now();
    auto stream = new EdgeStream(graph_path);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Stream loading time (in ms): " << duration.count());

    auto driver = new LiveGraphDriver();
    bool validate = result["validate"].as<bool>();
    driver->load_graph(stream, configuration().get_n_threads(), validate);
    

    // auto log_path = result["log_path"].as<string>();
    // start = high_resolution_clock::now();
    // auto update_stream = new UpdateStream(log_path);
    // end = high_resolution_clock::now();
    // duration = duration_cast<milliseconds>(end - start);
    // LOG("Update Stream loading time (in ms): " << duration.count());

    // start = high_resolution_clock::now();
    // driver->update_graph_batch(update_stream, configuration().get_n_threads());
    // end = high_resolution_clock::now();
    // duration = duration_cast<milliseconds>(end - start);
    // LOG("Update application time (in ms): " << duration.count());
}