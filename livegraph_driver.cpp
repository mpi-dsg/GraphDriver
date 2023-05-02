#include <iostream>
#include <omp.h>
#include <chrono>

#include "livegraph_driver.hpp"
#include "configuration.hpp"

using namespace lg;
using namespace std;
using namespace std::chrono;

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
        // tmp_cnt++;
        auto internal_id = tx.new_vertex();
        a->second = internal_id;
    }
    return a->second;
}

bool LiveGraphDriver::vertex_exists(uint64_t& external_id, Transaction& tx) {
    VertexDictionaryAccessor a;
    return ext2int.find(a, external_id);
}

void LiveGraphDriver::load_graph(EdgeStream* stream, int n_threads, bool validate) {
    auto sources = stream->get_sources();
    auto destinations = stream->get_destinations();

    auto start = high_resolution_clock::now();
    auto tx = graph->begin_batch_loader();
    LOG("Number of writer Threads: " << n_threads);
    # pragma omp parallel for num_threads(n_threads)
    for(uint64_t i = 0; i < sources.size(); i++) {
        auto src_internal = check_and_insert(sources[i], tx);
        auto dst_internal = check_and_insert(destinations[i], tx);
        // Undirected graph, so 2 directed edges
        tx.put_edge(src_internal, 0, dst_internal, "1");
        tx.put_edge(dst_internal, 0, src_internal, "1");
    }
    tx.commit();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Graph loading time (in ms): " << duration.count());
    LOG("Vertices Added: " << tmp_cnt);

    if(validate) validate_load_graph(stream, n_threads);
}

void LiveGraphDriver::update_graph(UpdateStream* update_stream, int n_threads = 1) {
    auto updates = update_stream->get_updates();

    LOG("Number of writer Threads: " << n_threads);
    # pragma omp parallel for num_threads(n_threads)
    for(uint64_t i = 0; i < updates.size(); i++) {
        auto update = updates[i];
        if(update->insert) add_edge(update->src, 0, update->dst);
        else remove_edge(update->src, 0, update->dst);

        tmp_cnt++;
    }

    cout << "Updates Applied: " << tmp_cnt << endl;
}

bool LiveGraphDriver::add_edge(uint64_t ext_id1, uint16_t label, uint64_t ext_id2) {
    bool done = false;
    // TODO: Confirm do we need to abort
    do {
        auto tx = graph->begin_transaction();
        try{
            if(!(vertex_exists(ext_id1, tx) && vertex_exists(ext_id2, tx))) { // Vertices DNE
                tx.abort();
                return false;
            }
            auto int_id1 = check_and_insert(ext_id1, tx);
            auto int_id2 = check_and_insert(ext_id2, tx);
            tx.put_edge(int_id1, label, int_id2, "");
            tx.put_edge(int_id2, label, int_id1, "");
            tx.commit();
            done = true;
        }
        catch(lg::Transaction::RollbackExcept& e) {
            tx.abort();
        }
    } while(!done);

    return true;
}

bool LiveGraphDriver::remove_edge(uint64_t ext_id1, uint16_t label, uint64_t ext_id2) {
    bool done = false;
    // TODO: Confirm do we need to abort
    do {
        auto tx = graph->begin_transaction();
        try{
            if(!(vertex_exists(ext_id1, tx) && vertex_exists(ext_id2, tx))) { // Vertices DNE
                tx.abort();
                return false;
            }
            auto int_id1 = check_and_insert(ext_id1, tx);
            auto int_id2 = check_and_insert(ext_id2, tx);
            tx.del_edge(int_id1, label, int_id2);
            tx.del_edge(int_id2, label, int_id1);
            tx.commit();
            done = true;
        }
        catch(lg::Transaction::RollbackExcept& e) {
            tx.abort();
        }
    } while(!done);

    return true;
}

void LiveGraphDriver::update_graph_batch(UpdateStream* update_stream, uint64_t batch_size, int n_threads = 1) {
    auto updates = update_stream->get_updates();

    LOG("Batch update with threads: " << n_threads);
    LOG("Batch update with batch_size: " << batch_size);

    // uint64_t batch_size = 1ull<<20;
    uint64_t num_batches = updates.size()/batch_size + (updates.size() % batch_size != 0);
    uint64_t applied_updates = 0;

    for(uint64_t b_no = 0; b_no < num_batches; b_no++){
        LOG("Batch: " << b_no);
        uint64_t start = applied_updates;
        uint64_t end = min(applied_updates + batch_size, updates.size());

        auto tx = graph->begin_batch_loader();
        # pragma omp parallel for num_threads(n_threads)
        for(uint64_t i = start; i < end; i++) {
            auto update = updates[i];
            if(update->insert) add_edge_batch(update->src, 0, update->dst, tx);
            else remove_edge_batch(update->src, 0, update->dst, tx);
            tmp_cnt++;
        }
        tx.commit();
        // graph->compact();

        applied_updates += batch_size;
    }

    cout << "Updates Applied: " << tmp_cnt << endl;

    // auto updates = update_stream->get_updates();
    // auto tx = graph->begin_batch_loader();

    // LOG("Batch update with threads: " << n_threads);
    // # pragma omp parallel for num_threads(n_threads)
    // for(uint64_t i = 0; i < updates.size(); i++) {
    //     auto update = updates[i];
    //     if(update->insert) add_edge_batch(update->src, 0, update->dst, tx);
    //     else remove_edge_batch(update->src, 0, update->dst, tx);
    //     tmp_cnt++;
    // }

    // tx.commit();

    // cout << "Updates Applied: " << tmp_cnt << endl;
}

bool LiveGraphDriver::add_edge_batch(uint64_t ext_id1, uint16_t label, uint64_t ext_id2, Transaction& tx) {
    // TODO: Only for single thread
    if(!(vertex_exists(ext_id1, tx) && vertex_exists(ext_id2, tx))) { // Vertices DNE
        tx.abort();
        LOG("WRONG");
        return false;
    }
    auto int_id1 = check_and_insert(ext_id1, tx);
    auto int_id2 = check_and_insert(ext_id2, tx);
    tx.put_edge(int_id1, label, int_id2, "");
    tx.put_edge(int_id2, label, int_id1, "");
    tx.commit();

    return true;
}

bool LiveGraphDriver::remove_edge_batch(uint64_t ext_id1, uint16_t label, uint64_t ext_id2, Transaction& tx) {
    // TODO: Only for single thread
    if(!(vertex_exists(ext_id1, tx) && vertex_exists(ext_id2, tx))) { // Vertices DNE
        tx.abort();
        LOG("WRONG");
        return false;
    }
    auto int_id1 = check_and_insert(ext_id1, tx);
    auto int_id2 = check_and_insert(ext_id2, tx);
    tx.del_edge(int_id1, label, int_id2);
    tx.del_edge(int_id2, label, int_id1);
    tx.commit();

    return true;
}

void LiveGraphDriver::validate_load_graph(EdgeStream* stream, int n_threads) {
    LOG("Validating Load Graph");
    auto sources = stream->get_sources();
    auto destinations = stream->get_destinations();

    auto tx = graph->begin_read_only_transaction();

    # pragma omp parallel for num_threads(n_threads)
    for(uint64_t i = 0; i < sources.size(); i++) {
        auto src = sources[i];
        auto dst = destinations[i];
        if(!(vertex_exists(src, tx) && vertex_exists(dst, tx))) { // Vertices DNE
            LOG("ERROR-no-vertex: " << src << dst);
        }
        auto src_internal = check_and_insert(src, tx);
        auto dst_internal = check_and_insert(dst, tx);
        // Undirected graph, so 2 directed edges
        auto data = tx.get_edge(src_internal, 0, dst_internal);
        if(data.size() <= 0) LOG("ERROR-no-edge: " << src << "->" << dst);
        data = tx.get_edge(dst_internal, 0, src_internal);
        if(data.size() <= 0) LOG("ERROR-no-edge: " << dst << "->" << src);
    }

    tx.abort();
    LOG("Validation finished");
}