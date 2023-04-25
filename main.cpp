#include <iostream>
#include <chrono>

#include "configuration.hpp"
#include "livegraph_driver.hpp"

using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[]){
    configuration().set_n_threads(argc, argv);

    auto driver = new LiveGraphDriver();

    auto path = "/home/hkatehar/gfe_driver/graph22/datasets/graph500-22.properties";
    auto start = high_resolution_clock::now();
    auto stream = new EdgeStream(path);
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<seconds>(end - start);
    LOG("Stream loaded in " << duration.count() << " seconds");

    start = high_resolution_clock::now();
    driver->load_graph(stream, configuration().get_n_threads());
    end = high_resolution_clock::now();
    duration = duration_cast<seconds>(end - start);
    LOG("Graph loaded in " << duration.count() << " seconds");
    
}