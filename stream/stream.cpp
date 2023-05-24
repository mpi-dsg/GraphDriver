#include <algorithm>
#include <random>

#include "stream.hpp"
#include "../reader/graph_reader.hpp"
#include "../configuration.hpp"

EdgeStream::EdgeStream(const string& path) {
    LOG("Using graph path: " << path);
    auto reader = new GraphReader(path);
    auto n_edges = reader->get_n_edges();
    
    uint64_t src = -1, des = -1;
    while(reader->read_edge(src, des)) {
        sources.push_back(src);
        destinations.push_back(des);
    }

    LOG("Permuting");
    permute(sources, destinations, generate_permutation_vector(sources.size()));
}

vector<uint64_t> EdgeStream::generate_permutation_vector(uint64_t size) {
    vector<uint64_t> permutation(size);
    iota(permutation.begin(), permutation.end(), 0);

    random_device rd;
    mt19937 rng(rd());
    shuffle(permutation.begin(), permutation.end(), rng);

    return permutation;
}

void EdgeStream::permute(vector<uint64_t>& array1, vector<uint64_t>& array2, const vector<uint64_t>& permutation) {
    vector<uint64_t> temp1(array1.size());
    vector<uint64_t> temp2(array2.size());

    for (uint64_t i = 0; i < permutation.size(); ++i) {
        temp1[i] = array1[permutation[i]];
        temp2[i] = array2[permutation[i]];
    }

    array1 = move(temp1);
    array2 = move(temp2);
}


EdgeStream::~EdgeStream() {}

vector<uint64_t> EdgeStream::get_sources() {
    return sources;
}

vector<uint64_t> EdgeStream::get_destinations() {
    return destinations;
}