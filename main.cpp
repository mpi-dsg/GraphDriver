#include <iostream>
#include <chrono>
#include <cxxopts.hpp>

#include "configuration.hpp"
#include "livegraph_driver.hpp"
#include "reader/log_reader.hpp"
#include "stream/update_stream.hpp"

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

    // auto log_reader = new LogReader(result["log_path"].as<string>());
    // uint64_t src, dst, cnt = 0, rem_cnt = 0, add_cnt = 0;
    // vector<uint64_t> sources, destinations;
    // vector<bool> is_addition;
    // double wt;
    // while(log_reader->read_edge(src, dst, wt)) {
    //     if(wt != 0) rem_cnt++;
    //     else add_cnt++;
    //     cnt++;
    //     sources.push_back(src);
    //     destinations.push_back(dst);
    //     is_addition.push_back(wt==0);
    // }
    // LOG(add_cnt);
    // LOG(rem_cnt);
    // LOG(cnt);

    uint64_t add = 0, rem = 0;
    auto update_stream = new UpdateStream(result["log_path"].as<string>());
    auto updates = update_stream->get_updates();
    for(auto x: updates) x->insert ? add++ : rem++;

    LOG(add);
    LOG(rem);
    LOG(updates.size());
}