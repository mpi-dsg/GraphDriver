#include <chrono>
#include <omp.h>

#include "data-structure/data_types.h"
#include "data-structure/EdgeDoesNotExistsPrecondition.h"
#include "sortledton_driver.hpp"
#include "../configuration.hpp"

using namespace std;
using namespace std::chrono;

SortledtonDriver::SortledtonDriver(): tm(32) {
    ds = new VersioningBlockedSkipListAdjacencyList(1024, 8, tm);
}

SortledtonDriver::~SortledtonDriver() {
    delete ds;
    ds = nullptr;
}

void SortledtonDriver::load_graph(EdgeStream& stream, int n_threads, bool validate) {
    auto sources = stream.get_sources();
    auto destinations = stream.get_destinations();

    auto start = high_resolution_clock::now();
    
    LOG("Number of writer Threads: " << n_threads);
    tm.reset_max_threads(n_threads);
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp critical
        {
            tm.register_thread(omp_get_thread_num());
        }
    }

    # pragma omp parallel for num_threads(n_threads) schedule(dynamic, 1024)
    for(uint64_t i = 0; i < sources.size(); i++) {
        // LOG(omp_get_thread_num() << ": Start");
        auto src = sources[i];
        auto dst = destinations[i];
        add_edge(src, dst);
        n_edges++;
        // LOG(omp_get_thread_num() << ": End");
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Graph loading time (in ms): " << duration.count());
    // LOG("Vertices Added: " << n_vertices);
    LOG("Edges Added: " << n_edges << "\n");

    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp critical
        {
            tm.deregister_thread(omp_get_thread_num());
        }
    }
}

bool SortledtonDriver::add_edge(uint64_t ext_id1, uint64_t ext_id2) {
    // LOG(ext_id1 << " " << ext_id2 << " - AddStart");
    thread_local optional <SnapshotTransaction> tx_o = nullopt;
    if (tx_o.has_value()) {
        tm.getSnapshotTransaction(ds, true, *tx_o);
    } else {
        tx_o = tm.getSnapshotTransaction(ds, true);
    }
    auto tx = *tx_o;

    tx.use_vertex_does_not_exists_semantics();

    tx.insert_vertex(ext_id1);
    tx.insert_vertex(ext_id2);
    
    bool inserted = true;
    try {
        tx.insert_edge({ext_id1, ext_id2}, (char *) "0", sizeof("0"));
        tx.insert_edge({ext_id2, ext_id1}, (char *) "0", sizeof("0"));\
        inserted &= tx.execute();
    }
    catch (EdgeExistsException &e) {
        inserted = false;
    }
   
    tm.transactionCompleted(tx);
    // LOG(ext_id1 << " " << ext_id2 << " - AddEnd");
    return inserted;

    // thread_local optional <SnapshotTransaction> tx_o = nullopt;
    // if (tx_o.has_value()) {
    // tm.getSnapshotTransaction(ds, true, *tx_o);
    // } else {
    // tx_o = tm.getSnapshotTransaction(ds, true);
    // }
    // auto tx = *tx_o;

    // VertexExistsPrecondition pre_v1(ext_id1);
    // tx.register_precondition(&pre_v1);
    // VertexExistsPrecondition pre_v2(ext_id2);
    // tx.register_precondition(&pre_v2);
    // // Even in the undirected case, we need to check only for the existence of one edge direction to ensure consistency.
    // EdgeDoesNotExistsPrecondition pre_e({ext_id1, ext_id2});
    // tx.register_precondition(&pre_e);

    // // test
    // bool inserted = true;
    // try {
    // tx.insert_edge({ext_id1, ext_id2}, (char *) 0, sizeof(0));
    // tx.insert_edge({ext_id2, ext_id1}, (char *) 0, sizeof(0));
    // inserted &= tx.execute();
    // } catch (VertexDoesNotExistsException &e) {
    // inserted = false;
    // } catch (EdgeExistsException &e) {
    // inserted = false;
    // }
    // tm.transactionCompleted(tx);
    // return inserted;
}

bool SortledtonDriver::remove_edge(uint64_t ext_id1, uint64_t ext_id2) {
    LOG(ext_id1 << " " << ext_id2 << " - RemoveStart");
    thread_local optional <SnapshotTransaction> tx_o = nullopt;
    if (tx_o.has_value()) {
        tm.getSnapshotTransaction(ds, true, *tx_o);
    } else {
        tx_o = tm.getSnapshotTransaction(ds, true);
    }
    auto tx = *tx_o;
    
    tx.delete_edge({ext_id1, ext_id2});
    tx.delete_edge({ext_id2, ext_id1});
    
    bool removed = true;
    removed &= tx.execute();
   
    tm.transactionCompleted(tx);
    LOG(ext_id1 << " " << ext_id2 << " - RemoveEnd");
    return removed;
}

void SortledtonDriver::load_graph_batch(EdgeStream& stream, int n_threads, bool validate) {
    auto sources = stream.get_sources();
    auto destinations = stream.get_destinations();

    auto start = high_resolution_clock::now();
    
    LOG("Number of writer Threads: " << n_threads);
    tm.reset_max_threads(n_threads);
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp critical
        {
            tm.register_thread(omp_get_thread_num());
        }
    }

    uint64_t batch_size = 10000;
    uint64_t num_batches = sources.size()/batch_size + (sources.size() % batch_size != 0);

    atomic<bool> inserted = true;

    # pragma omp parallel for num_threads(n_threads)
    for(uint64_t b_no = 0; b_no < num_batches; b_no++) {
        uint64_t start = b_no*batch_size; 
        uint64_t end = min(start + batch_size, sources.size());

        thread_local optional <SnapshotTransaction> tx_o = nullopt;
        if (tx_o.has_value()) {
            tm.getSnapshotTransaction(ds, true, *tx_o);
        } 
        else {
            tx_o = tm.getSnapshotTransaction(ds, true);
        }
        auto tx = *tx_o;

        tx.use_vertex_does_not_exists_semantics();

        for(uint64_t i = start; i < end; i++){
            auto src = sources[i];
            auto dst = destinations[i];
            add_edge_batch(src, dst, tx);
            n_edges++;
        }
        inserted.store(inserted.load() && tx.execute());
        // inserted &= tx.execute();
        tm.transactionCompleted(tx);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Graph loading time (in ms): " << duration.count());
    // LOG("Vertices Added: " << n_vertices);
    LOG("Edges Added: " << n_edges << "\n");
}

void SortledtonDriver::add_edge_batch(uint64_t ext_id1, uint64_t ext_id2, SnapshotTransaction& tx) {
    tx.insert_vertex(ext_id1);
    tx.insert_vertex(ext_id2);
    
    tx.insert_edge({ext_id1, ext_id2}, (char *) "0", sizeof("0"));
    tx.insert_edge({ext_id2, ext_id1}, (char *) "0", sizeof("0"));
}

void SortledtonDriver::update_graph(UpdateStream& update_stream, int n_threads) {
    auto updates = update_stream.get_updates();

    // tm.reset_max_threads(n_threads);
    // #pragma omp parallel num_threads(n_threads)
    // {
    //     #pragma omp critical
    //     {
    //         tm.register_thread(omp_get_thread_num());
    //     }
    // }

    // LOG("Number of writer Threads: " << n_threads);
    // # pragma omp parallel num_threads(n_threads)
    // {
    //     LOG("New");
    //     int thread_count = omp_get_num_threads();
    //     int thread_id = omp_get_thread_num();
    //     for(uint64_t i = 0; i < updates.size(); i++) {
    //         // LOG(i << " Start");
    //         auto update = updates[i];
    //         if(static_cast<int>( (update.src + update.dst) % thread_count ) == thread_id) {
    //             if(update.insert) add_edge(update.src, update.dst);
    //             else remove_edge(update.src, update.dst);
    //             updates_applied++;
    //             LOG(i);
    //         }
    //         // LOG(i << " End");
    //     }
    // }
    tm.register_thread(omp_get_thread_num());
    for(uint64_t i = 0; i < updates.size(); i++) {
        LOG(i << " Start");
        auto update = updates[i];
        // if(!update.insert) remove_edge(update.src, update.dst);
        if(update.insert){
            LOG("Add: " << add_edge(update.src, update.dst));
        } 
        else LOG("Remove: " << remove_edge(update.src, update.dst));
        updates_applied++;
        LOG(i);
        LOG(i << " End");
    }
    tm.deregister_thread(omp_get_thread_num());
    LOG("Total Updates Applied: " << updates_applied);
}
