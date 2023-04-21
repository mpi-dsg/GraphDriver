#include <iostream>

#include "livegraph_driver.hpp"

using namespace lg;
using namespace std;

LiveGraphDriver::LiveGraphDriver() {
    graph = new Graph();
    std::cout << "2323" << std::endl;
}

Graph* LiveGraphDriver::get_graph() {
    return graph;
}

void LiveGraphDriver::load_graph(string path) {
    
}