#include <iostream>
#include <omp.h>

#include "livegraph_driver.hpp"
#include "configuration.hpp"

using namespace lg;
using namespace std;

LiveGraphDriver::LiveGraphDriver() {
    graph = new Graph();
}

Graph* LiveGraphDriver::get_graph() {
    return graph;
}

uint64_t LiveGraphDriver::check_and_insert(uint64_t& external_id, Transaction& tx) {
    VertexDictionaryAccessor a;
    const auto isNew = ext2int.insert(a, external_id);
    // Create new vertex if new entry to hashmap
    if(isNew) {
        tmp_cnt++;
        auto internal_id = tx.new_vertex();
        a->second = internal_id;
    }
    return a->second;
}

void LiveGraphDriver::load_graph(EdgeStream* stream, int n_threads = 1) {
    auto sources = stream->get_sources();
    auto destinations = stream->get_destinations();

    auto tx = graph->begin_batch_loader();
    LOG("Number of writer Threads: " << n_threads);
    # pragma omp parallel for num_threads(n_threads)
    for(uint64_t i = 0; i < sources.size(); i++) {
        tx.put_edge(check_and_insert(sources[i], tx), 0, check_and_insert(destinations[i], tx), "");
    }
    tx.commit();

    cout << "Vertices Added: " << tmp_cnt << endl;
}