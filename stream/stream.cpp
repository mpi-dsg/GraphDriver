#include "stream.hpp"
#include "../reader/graph_reader.hpp"

EdgeStream::EdgeStream(const string& path) {
    auto reader = new GraphReader(path);
    auto n_edges = reader->get_n_edges();
    
    uint64_t src = -1, des = -1;
    while(reader->read_edge(src, des)) {
        sources.push_back(src);
        destinations.push_back(des);
    }
}

vector<uint64_t> EdgeStream::get_sources() {
    return sources;
}

vector<uint64_t> EdgeStream::get_destinations() {
    return destinations;
}