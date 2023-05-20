#ifndef LIVEGRAPH_DRIVER
#define LIVEGRAPH_DRIVER

#include <map>
#include <atomic>

#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_vector.h"
#include "livegraph/livegraph.hpp"
#include "reader/graph_reader.hpp"
#include "stream/stream.hpp"
#include "stream/update_stream.hpp"

using namespace lg;
using namespace std;

typedef tbb::concurrent_hash_map<uint64_t, uint64_t> VertexDictionary;
typedef VertexDictionary::accessor VertexDictionaryAccessor;

class LiveGraphDriver{
public:
    atomic<uint64_t> updates_applied {0};

    LiveGraphDriver();
    ~LiveGraphDriver();

    Graph* get_graph();
    void load_graph(EdgeStream& stream, int n_threads = 1, bool validate = false);
    void update_graph(UpdateStream& update_stream, int n_threads);
    void update_graph_batch(UpdateStream& update_stream, uint64_t batch_size, int n_threads = 1, bool log = true);

    void start_updates(UpdateStream& update_stream, int n_threads = 1);
    void add_to_buffer(EdgeUpdate& update);
    void apply_updates();
    void update_graph_batch_from_queue(tbb::concurrent_vector<EdgeUpdate>* inactive_buffer, uint64_t batch_size, int n_threads = 1);
    bool stop_sequential();

    unique_ptr<int64_t[]> execute_bfs(uint64_t ext_root = 1, int alpha = 15, int beta = 18);
    void print_bfs_output(int64_t* dist, uint64_t max_vertex_id, Transaction& tx);
private:
    Graph* graph;
    VertexDictionary ext2int_map;
    atomic<uint64_t> tmp_cnt {0} ;
    atomic<uint64_t> n_vertices {0};
    atomic<uint64_t> n_edges {0};
    
    tbb::concurrent_vector<EdgeUpdate>* active_buffer;
    tbb::concurrent_vector<EdgeUpdate>* inactive_buffer;
    atomic<bool> all_updates_added_to_buffer = false;

    bool vertex_exists(uint64_t& external_id, Transaction& tx);
    uint64_t ext2int(uint64_t& external_id, Transaction& tx);
    uint64_t ext2int_with_insert(uint64_t& external_id, Transaction& tx);
    uint64_t int2ext(uint64_t& internal_id, Transaction& tx);

    bool add_edge(uint64_t ext_id1, uint16_t label, uint64_t ext_id2);
    bool remove_edge(uint64_t ext_id1, uint16_t label, uint64_t ext_id2);

    bool add_edge_batch(uint64_t ext_id1, uint16_t label, uint64_t ext_id2, Transaction& tx);
    bool remove_edge_batch(uint64_t ext_id1, uint16_t label, uint64_t ext_id2, Transaction& tx);

    void validate_load_graph(EdgeStream& stream, int n_threads = 1);
};
#endif