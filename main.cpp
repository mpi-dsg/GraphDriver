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
        ("g,graph_path", "Path to graph properties", cxxopts::value<std::string>())
        ("l,log_path", "Path to graph update log", cxxopts::value<std::string>())
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

    // auto path = result["graph_path"].as<string>();
    // auto start = high_resolution_clock::now();
    // auto stream = new EdgeStream(path);
    // auto end = high_resolution_clock::now();

    // auto duration = duration_cast<milliseconds>(end - start);
    // LOG("Stream loading time (in ms): " << duration.count());

    // auto driver = new LiveGraphDriver();
    // start = high_resolution_clock::now();
    // driver->load_graph(stream, configuration().get_n_threads());
    // end = high_resolution_clock::now();
    // duration = duration_cast<milliseconds>(end - start);
    // LOG("Graph loading time (in ms): " << duration.count());

    auto log_reader = new LogReader(result["log_path"].as<string>());
    uint64_t src, des, cnt = 0;
    double wt;
    while(log_reader->read_edge(src, des, wt)) {
        cnt++;
    }

    LOG(cnt);
    
}