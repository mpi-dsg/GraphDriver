#include <iostream>
#include <omp.h>
#include <thread>
#include <chrono>
#include <unordered_set>

#include "livegraph_driver.hpp"
#include "configuration.hpp"
#include "third_party_lib/gapbs/gapbs.hpp"

using namespace lg;
using namespace std;
using namespace std::chrono;

LiveGraphDriver::LiveGraphDriver() {
    graph = new Graph();
    active_buffer = new tbb::concurrent_vector<EdgeUpdate>();
    inactive_buffer = new tbb::concurrent_vector<EdgeUpdate>();
}

LiveGraphDriver::~LiveGraphDriver() {
    delete active_buffer;
    delete inactive_buffer;
}

Graph* LiveGraphDriver::get_graph() {
    return graph;
}

uint64_t LiveGraphDriver::int2ext(uint64_t& internal_id, Transaction& tx) {
    string_view payload = tx.get_vertex(internal_id);
    if(payload.empty()){ // the vertex does not exist
        return numeric_limits<uint64_t>::max();
    } else {
        return *(reinterpret_cast<const uint64_t*>(payload.data()));
    }
}

uint64_t LiveGraphDriver::ext2int(uint64_t& external_id, Transaction& tx) {
    VertexDictionaryAccessor a;
    bool exists = ext2int_map.find(a, external_id);
    if(!exists) return numeric_limits<uint64_t>::max(); 
    return a->second;
}

uint64_t LiveGraphDriver::ext2int_with_insert(uint64_t& external_id, Transaction& tx) {
    VertexDictionaryAccessor a;
    const auto isNew = ext2int_map.insert(a, external_id);
    // Create new vertex if new entry to hashmap
    if(isNew) {
        n_vertices++;
        auto internal_id = tx.new_vertex();
        string_view data { (char*) &external_id, sizeof(external_id) };
        tx.put_vertex(internal_id, data);
        a->second = internal_id;
    }
    return a->second;
}

bool LiveGraphDriver::vertex_exists(uint64_t& external_id, Transaction& tx) {
    VertexDictionaryAccessor a;
    return ext2int_map.find(a, external_id);
}

void LiveGraphDriver::load_graph(EdgeStream& stream, int n_threads, bool validate) {
    auto sources = stream.get_sources();
    auto destinations = stream.get_destinations();

    auto start = high_resolution_clock::now();
    auto tx = graph->begin_batch_loader();
    LOG("Number of writer Threads: " << n_threads);
    # pragma omp parallel for num_threads(n_threads)
    for(uint64_t i = 0; i < sources.size(); i++) {
        auto src_internal = ext2int_with_insert(sources[i], tx);
        auto dst_internal = ext2int_with_insert(destinations[i], tx);
        // Undirected graph, so 2 directed edges
        tx.put_edge(src_internal, 0, dst_internal, "1");
        tx.put_edge(dst_internal, 0, src_internal, "1");
        n_edges++;
    }
    tx.commit();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    LOG("Graph loading time (in ms): " << duration.count());
    LOG("Vertices Added: " << n_vertices);
    LOG("Edges Added: " << n_edges << "\n");

    if(validate) validate_load_graph(stream, n_threads);
}

void LiveGraphDriver::update_graph(UpdateStream& update_stream, int n_threads = 1) {
    auto updates = update_stream.get_updates();

    LOG("Number of writer Threads: " << n_threads);
    # pragma omp parallel for num_threads(n_threads)
    for(uint64_t i = 0; i < updates.size(); i++) {
        auto update = updates[i];
        if(update.insert) add_edge(update.src, 0, update.dst);
        else remove_edge(update.src, 0, update.dst);

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
            auto int_id1 = ext2int(ext_id1, tx);
            auto int_id2 = ext2int(ext_id2, tx);
            tx.put_edge(int_id1, label, int_id2, "");
            tx.put_edge(int_id2, label, int_id1, "");
            tx.commit();
            done = true;
            n_edges++;
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
            auto int_id1 = ext2int(ext_id1, tx);
            auto int_id2 = ext2int(ext_id2, tx);
            tx.del_edge(int_id1, label, int_id2);
            tx.del_edge(int_id2, label, int_id1);
            tx.commit();
            done = true;
            n_edges--;
        }
        catch(lg::Transaction::RollbackExcept& e) {
            tx.abort();
        }
    } while(!done);

    return true;
}

void LiveGraphDriver::update_graph_batch(UpdateStream& update_stream, uint64_t batch_size, int n_threads, bool log) {
    auto updates = update_stream.get_updates();

    if(log) {
        LOG("Batch update with threads: " << n_threads);
        LOG("Batch update with batch_size: " << batch_size);
    }

    uint64_t num_batches = updates.size()/batch_size + (updates.size() % batch_size != 0);
    // uint64_t applied_updates = 0;
    LOG("Size: " << updates.size());
    LOG("Num batches:" << num_batches);

    uint64_t done = 0;
    for(uint64_t b_no = 0; b_no < num_batches; b_no++){
        if(log) LOG("Batch: " << b_no);
        uint64_t start = done;
        uint64_t end = min(done + batch_size, updates.size());

        auto tx = graph->begin_batch_loader();
        # pragma omp parallel for num_threads(n_threads)
        for(uint64_t i = start; i < end; i++) {
            auto update = updates[i];
            updates_applied++;
            if(update.insert) add_edge_batch(update.src, 0, update.dst, tx);
            else remove_edge_batch(update.src, 0, update.dst, tx);
        }
        tx.commit();
        // graph->compact();

        done += batch_size;
    }

    LOG("Total Updates Applied: " << updates_applied);
}

bool LiveGraphDriver::add_edge_batch(uint64_t ext_id1, uint16_t label, uint64_t ext_id2, Transaction& tx) {
    if(!(vertex_exists(ext_id1, tx) && vertex_exists(ext_id2, tx))) { // Vertices DNE
        tx.abort();
        LOG("WRONG");
        return false;
    }
    auto int_id1 = ext2int(ext_id1, tx);
    auto int_id2 = ext2int(ext_id2, tx);
    tx.put_edge(int_id1, label, int_id2, "");
    tx.put_edge(int_id2, label, int_id1, "");
    n_edges++;
    return true;
}

bool LiveGraphDriver::remove_edge_batch(uint64_t ext_id1, uint16_t label, uint64_t ext_id2, Transaction& tx) {
    if(!(vertex_exists(ext_id1, tx) && vertex_exists(ext_id2, tx))) { // Vertices DNE
        tx.abort();
        LOG("WRONG");
        return false;
    }
    auto int_id1 = ext2int(ext_id1, tx);
    auto int_id2 = ext2int(ext_id2, tx);
    tx.del_edge(int_id1, label, int_id2);
    tx.del_edge(int_id2, label, int_id1);
    n_edges--;
    return true;
}

void LiveGraphDriver::validate_load_graph(EdgeStream& stream, int n_threads) {
    LOG("Validating Load Graph");
    auto sources = stream.get_sources();
    auto destinations = stream.get_destinations();

    auto tx = graph->begin_read_only_transaction();

    # pragma omp parallel for num_threads(n_threads)
    for(uint64_t i = 0; i < sources.size(); i++) {
        auto src = sources[i];
        auto dst = destinations[i];
        if(!(vertex_exists(src, tx) && vertex_exists(dst, tx))) { // Vertices DNE
            LOG("ERROR-no-vertex: " << src << dst);
        }
        auto src_internal = ext2int(src, tx);
        auto dst_internal = ext2int(dst, tx);
        // Undirected graph, so 2 directed edges
        auto data = tx.get_edge(src_internal, 0, dst_internal);
        if(data.size() <= 0) LOG("ERROR-no-edge: " << src << "->" << dst);
        data = tx.get_edge(dst_internal, 0, src_internal);
        if(data.size() <= 0) LOG("ERROR-no-edge: " << dst << "->" << src);
    }

    tx.abort();
    LOG("Validation finished");
}

/*
    Sequential Experiment Methods
*/
void LiveGraphDriver::add_to_buffer(EdgeUpdate& update) {
    // while(!can_add) {}
    active_buffer->push_back(update);
}

void LiveGraphDriver::start_updates(UpdateStream& update_stream, int n_threads) {
    auto updates = update_stream.get_updates();
    auto rate = configuration().get_rate();
    LOG("Starting updates with rate: " << rate);

    uint64_t done = 0, start, end;
    while(done < updates.size()) {
        start = done;
        end = done + rate;
        auto t_start = high_resolution_clock::now();
        for(uint64_t i = start; i < end && i < updates.size(); i++) {
            auto update = updates[i];
            add_to_buffer(update);
        }
        done = end;
        auto t_end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(t_end - t_start);
        auto sleep_duration = 1000-duration.count();
        if(sleep_duration < 0) LOG("Warning: Invalid sleep");
        // LOG("Sleep: " << sleep_duration);
        this_thread::sleep_for(chrono::milliseconds(sleep_duration));
    }
    all_updates_added_to_buffer = true;
    cout << "Updates added to buffer: " << done << endl;
}

void LiveGraphDriver::apply_updates() {
    LOG("Applying");
    swap(active_buffer, inactive_buffer);


    auto updates = *inactive_buffer;
    auto size = updates.size();

    uint64_t batch_size = configuration().get_batch_size();
    int n_threads = configuration().get_n_threads();

    uint64_t num_batches = size/batch_size + (size % batch_size != 0);
    
    // LOG("Size: " << size);
    // LOG("Num batches:" << num_batches);

    uint64_t done = 0;
    for(uint64_t b_no = 0; b_no < num_batches; b_no++){
        uint64_t start = done;
        uint64_t end = min(done + batch_size, size);

        auto tx = graph->begin_batch_loader();
        # pragma omp parallel for num_threads(n_threads)
        for(uint64_t i = start; i < end; i++) {
            auto update = updates[i];
            updates_applied++;
            if(update.insert) add_edge_batch(update.src, 0, update.dst, tx);
            else remove_edge_batch(update.src, 0, update.dst, tx);
        }
        tx.commit();
        // graph->compact();

        done += batch_size;
    }

    // LOG("Total Updates Applied: " << updates_applied);
    
    delete inactive_buffer;
    inactive_buffer = new tbb::concurrent_vector<EdgeUpdate>();

}

// void LiveGraphDriver::update_graph_batch_from_queue(tbb::concurrent_vector<EdgeUpdate>* inactive_buffer, uint64_t batch_size, int n_threads) {
//     UpdateStream update_stream;
//     while(!inactive_buffer->empty()) {
//         EdgeUpdate update;
//         if(inactive_buffer->try_pop(update)) {
//             update_stream.add_update(update);
//         }
//     }
//     update_graph_batch(update_stream, batch_size, n_threads, false /* No logging on every step*/ );
// }

bool LiveGraphDriver::stop_sequential() {
    return all_updates_added_to_buffer && active_buffer->empty() && inactive_buffer->empty();
}

/*
    BFS
*/
static
unique_ptr<int64_t[]> do_bfs_init_distances(Transaction& tx, uint64_t max_vertex_id) {
    unique_ptr<int64_t[]> distances{ new int64_t[max_vertex_id] };
    #pragma omp parallel for
    for (uint64_t n = 0; n < max_vertex_id; n++){
        if(tx.get_vertex(n).empty()){ // the vertex does not exist
            distances[n] = numeric_limits<int64_t>::max();
        } else { // the vertex exists
            // Retrieve the out degree for the vertex n
            uint64_t out_degree = 0;
            auto iterator = tx.get_edges(n, /* label */ 0);
            while(iterator.valid()){
                out_degree++;
                iterator.next();
            }

            distances[n] = out_degree != 0 ? - out_degree : -1;
        }
    }

    return distances;
}

static
void do_bfs_QueueToBitmap(uint64_t max_vertex_id, const gapbs::SlidingQueue<int64_t> &queue, gapbs::Bitmap &bm) {
    #pragma omp parallel for
    for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
        int64_t u = *q_iter;
        bm.set_bit_atomic(u);
    }
}

static
void do_bfs_BitmapToQueue(uint64_t max_vertex_id, const gapbs::Bitmap &bm, gapbs::SlidingQueue<int64_t> &queue) {
    #pragma omp parallel
    {
        gapbs::QueueBuffer<int64_t> lqueue(queue);
        #pragma omp for
        for (uint64_t n=0; n < max_vertex_id; n++)
            if (bm.get_bit(n))
                lqueue.push_back(n);
        lqueue.flush();
    }
    queue.slide_window();
}

static
int64_t do_bfs_BUStep(Transaction& tx, uint64_t max_vertex_id, int64_t* distances, int64_t distance, gapbs::Bitmap &front, gapbs::Bitmap &next) {
    int64_t awake_count = 0;
    next.reset();

    #pragma omp parallel for schedule(dynamic, 1024) reduction(+ : awake_count)
    for (uint64_t u = 0; u < max_vertex_id; u++) {
        if(distances[u] == numeric_limits<int64_t>::max()) continue; // the vertex does not exist
        // COUT_DEBUG_BFS("explore: " << u << ", distance: " << distances[u]);

        if (distances[u] < 0){ // the node has not been visited yet
            auto iterator = tx.get_edges(u, /* label */ 0); 

            while(iterator.valid()){
                uint64_t dst = iterator.dst_id();
                // COUT_DEBUG_BFS("\tincoming edge: " << dst);

                if(front.get_bit(dst)) {
                    // COUT_DEBUG_BFS("\t-> distance updated to " << distance << " via vertex #" << dst);
                    distances[u] = distance; // on each BUStep, all nodes will have the same distance
                    awake_count++;
                    next.set_bit(u);
                    break;
                }

                iterator.next();
            }
        }
    }

    return awake_count;
}

static
int64_t do_bfs_TDStep(Transaction& tx, uint64_t max_vertex_id, int64_t* distances, int64_t distance, gapbs::SlidingQueue<int64_t>& queue) {
    int64_t scout_count = 0;

    #pragma omp parallel reduction(+ : scout_count)
    {
        gapbs::QueueBuffer<int64_t> lqueue(queue);

        #pragma omp for schedule(dynamic, 64)
        for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
            int64_t u = *q_iter;
            // COUT_DEBUG_BFS("explore: " << u);
            auto iterator = tx.get_edges(u, /* label */ 0);
            while(iterator.valid()){
                uint64_t dst = iterator.dst_id();
                // COUT_DEBUG_BFS("\toutgoing edge: " << dst);

                int64_t curr_val = distances[dst];
                if (curr_val < 0 && gapbs::compare_and_swap(distances[dst], curr_val, distance)) {
                    // COUT_DEBUG_BFS("\t-> distance updated to " << distance << " via vertex #" << dst);
                    lqueue.push_back(dst);
                    scout_count += -curr_val;
                }

                iterator.next();
            }
        }

        lqueue.flush();
    }

    return scout_count;
}

unique_ptr<int64_t[]> LiveGraphDriver::execute_bfs(uint64_t ext_root, int alpha, int beta) {
    int64_t edges_to_check = n_edges;
    int64_t num_vertices = n_vertices;
    uint64_t max_vertex_id = graph->get_max_vertex_id();

    auto tx = graph->begin_read_only_transaction();
    uint64_t root = ext2int_with_insert(ext_root, tx);

    unique_ptr<int64_t[]> ptr_distances = do_bfs_init_distances(tx, max_vertex_id);
    int64_t* __restrict distances = ptr_distances.get();
    distances[root] = 0;

    gapbs::SlidingQueue<int64_t> queue(max_vertex_id);
    queue.push_back(root);
    queue.slide_window();
    gapbs::Bitmap curr(max_vertex_id);
    curr.reset();
    gapbs::Bitmap front(max_vertex_id);
    front.reset();

    int64_t scout_count = 0;
    { // retrieve the out degree of the root
        auto iterator = tx.get_edges(root, 0);
        while(iterator.valid()){ scout_count++; iterator.next(); }
    }
    int64_t distance = 1; // current distance

    while (!queue.empty()) {

        if (scout_count > edges_to_check / alpha) {
            int64_t awake_count, old_awake_count;
            do_bfs_QueueToBitmap(max_vertex_id, queue, front);
            awake_count = queue.size();
            queue.slide_window();
            do {
                old_awake_count = awake_count;
                awake_count = do_bfs_BUStep(tx, max_vertex_id, distances, distance, front, curr);
                front.swap(curr);
                distance++;
            } while ((awake_count >= old_awake_count) || (awake_count > (int64_t) num_vertices / beta));
            do_bfs_BitmapToQueue(max_vertex_id, front, queue);
            scout_count = 1;
        } else {
            edges_to_check -= scout_count;
            scout_count = do_bfs_TDStep(tx, max_vertex_id, distances, distance, queue);
            queue.slide_window();
            distance++;
        }
    }

    // print_bfs_output(ptr_distances.get(), get_graph()->get_max_vertex_id(), tx);
    tx.abort();

    return ptr_distances;
}

void LiveGraphDriver::print_bfs_output(int64_t* dist, uint64_t max_vertex_id, Transaction& tx){
    for(uint64_t logical_id = 0; logical_id < max_vertex_id; logical_id++){
        uint64_t external_id = int2ext(logical_id, tx);
        if(external_id == numeric_limits<uint64_t>::max()) { // the vertex does not exist
            LOG(external_id << ": " << -1);
        } else {
            LOG(external_id << ": " << dist[logical_id]);
        }
    }
}


uint64_t LiveGraphDriver::execute_tc() {
    auto transaction = graph->begin_read_only_transaction();
    uint64_t max_vertex_id = graph->get_max_vertex_id();

    uint64_t tc = 0;
    unique_ptr<uint32_t[]> ptr_degrees_out { new uint32_t[max_vertex_id] };
    uint32_t* __restrict degrees_out = ptr_degrees_out.get();

    LOG("In TC");
    omp_set_num_threads(2048);
    // precompute the degrees of the vertices
    #pragma omp parallel for schedule(dynamic, 4096)
    for(uint64_t v = 0; v < max_vertex_id; v++){
        bool vertex_exists = !transaction.get_vertex(v).empty();
        if(vertex_exists){ // out degree, restrict the scope
            uint32_t count = 0;
            auto iterator = transaction.get_edges(v, 0);
            while(iterator.valid()){ count ++; iterator.next(); }
            degrees_out[v] = count; 
        }
    }

    // LOG("In TC2");
    // LOG(max_vertex_id);
    #pragma omp parallel for reduction(+:tc) schedule(dynamic, 64)
    for(uint64_t v = 0; v < max_vertex_id; v++){
        if(degrees_out[v] == numeric_limits<uint32_t>::max()) continue; // the vertex does not exist

        // LOG("> Node " << v);

        uint64_t num_triangles = 0; // number of triangles found so far for the node v

        // Cfr. Spec v.0.9.0 pp. 15: "If the number of neighbors of a vertex is less than two, its coefficient is defined as zero"
        uint64_t v_degree_out = degrees_out[v];
        if(v_degree_out < 2) continue;

        // Build the list of neighbours of v
        unordered_set<uint64_t> neighbours;

        { // Fetch the list of neighbours of v
            auto iterator1 = transaction.get_edges(v, 0);
            while(iterator1.valid()){
                uint64_t u = iterator1.dst_id();
                neighbours.insert(u);
                iterator1.next();
            }
        }
        // LOG("> Node " << v << "Nbrs done");
        // again, visit all neighbours of v
        auto iterator1 = transaction.get_edges(v, /* label */ 0);
        while(iterator1.valid()){
            uint64_t u = iterator1.dst_id();
            if(u < v) {
                iterator1.next();
                continue;
            } // skip vertex with smaller id

            auto iterator2 = transaction.get_edges(u, /* label */ 0);
            while(iterator2.valid()){
                uint64_t w = iterator2.dst_id();
                if(w < u) {
                    iterator2.next();
                    continue;
                } // skip vertex with smaller id

                // check whether it's also a neighbour of v
                if(neighbours.count(w) == 1){
                    // COUT_DEBUG_TC("Triangle found " << v << " - " << u << " - " << w);
                    num_triangles++;
                }

                iterator2.next();
            }

            iterator1.next();
        }

        tc += num_triangles;
    }
    
    LOG("TC: " << tc);
    transaction.abort();

    return tc;
}

uint64_t LiveGraphDriver::execute_label_tc() {
    auto transaction = graph->begin_read_only_transaction();
    uint64_t max_vertex_id = graph->get_max_vertex_id();

    uint32_t LABEL_COUNT = 8;
    LOG("In label TC");
    for(uint32_t i_label = 0; i_label < LABEL_COUNT; i_label++){
        uint32_t tc = 0;
        unique_ptr<uint32_t[]> ptr_degrees_out { new uint32_t[max_vertex_id] };
        uint32_t* __restrict degrees_out = ptr_degrees_out.get();
        
        // precompute the degrees of the vertices
        #pragma omp parallel for schedule(dynamic, 4096)
        for(uint64_t v = 0; v < max_vertex_id; v++){
            bool vertex_exists = !transaction.get_vertex(v).empty();
            if(vertex_exists){ // out degree, restrict the scope
                uint32_t count = 0;
                auto iterator = transaction.get_edges(v, 0);
                while(iterator.valid()){ count ++; iterator.next(); }
                degrees_out[v] = count; 
            }
        }

        // LOG("In TC2");
        // LOG(max_vertex_id);
        #pragma omp parallel for reduction(+:tc) schedule(dynamic, 64)
        for(uint64_t v = 0; v < max_vertex_id; v++){
            uint64_t ext_v = int2ext(v, transaction);
            if((degrees_out[v] == numeric_limits<uint32_t>::max()) || (ext_v % LABEL_COUNT != i_label)) continue; // the vertex does not exist

            // LOG("> Node " << v);

            uint64_t num_triangles = 0; // number of triangles found so far for the node v

            // Cfr. Spec v.0.9.0 pp. 15: "If the number of neighbors of a vertex is less than two, its coefficient is defined as zero"
            uint64_t v_degree_out = degrees_out[v];
            if(v_degree_out < 2) continue;

            // Build the list of neighbours of v
            unordered_set<uint64_t> neighbours;

            { // Fetch the list of neighbours of v
                auto iterator1 = transaction.get_edges(v, 0);
                while(iterator1.valid()){
                    uint64_t u = iterator1.dst_id();
                    neighbours.insert(u);
                    iterator1.next();
                }
            }
            // LOG("> Node " << v << "Nbrs done");
            // again, visit all neighbours of v
            auto iterator1 = transaction.get_edges(v, /* label */ 0);
            while(iterator1.valid()){
                uint64_t u = iterator1.dst_id();
                uint64_t ext_u = int2ext(u, transaction);
                if((u < v) || (ext_u % LABEL_COUNT != i_label)) {
                    iterator1.next();
                    continue;
                } // skip vertex with smaller id or different label

                auto iterator2 = transaction.get_edges(u, /* label */ 0);
                while(iterator2.valid()){
                    uint64_t w = iterator2.dst_id();
                    uint64_t ext_w = int2ext(w, transaction);
                    if((w < u) || (ext_w % LABEL_COUNT != i_label)) {
                        iterator2.next();
                        continue;
                    } // skip vertex with smaller id

                    // check whether it's also a neighbour of v
                    if(neighbours.count(w) == 1){
                        // LOG("Triangle found " << v << " - " << u << " - " << w);
                        num_triangles++;
                    }

                    iterator2.next();
                }

                iterator1.next();
            }

            tc += num_triangles;
        }
        
        LOG("Labeled TC " << i_label << ": " << tc);
    }  

    return 1;
}