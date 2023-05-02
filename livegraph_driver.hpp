#include <map>
#include <atomic>

#include "tbb/concurrent_hash_map.h"
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
    LiveGraphDriver();
    ~LiveGraphDriver();

    Graph* get_graph();
    void load_graph(EdgeStream* stream, int n_threads = 1, bool validate = false);
    void update_graph(UpdateStream* update_stream, int n_threads);
    void update_graph_batch(UpdateStream* update_stream, uint64_t batch_size, int n_threads);

private:
    Graph* graph;
    map<uint64_t, uint64_t> int2ext;
    VertexDictionary ext2int;
    atomic<uint64_t> tmp_cnt {0} ;
    bool vertex_exists(uint64_t& external_id, Transaction& tx);
    uint64_t check_and_insert(uint64_t& external_id, Transaction& tx);

    bool add_edge(uint64_t ext_id1, uint16_t label, uint64_t ext_id2);
    bool remove_edge(uint64_t ext_id1, uint16_t label, uint64_t ext_id2);

    bool add_edge_batch(uint64_t ext_id1, uint16_t label, uint64_t ext_id2, Transaction& tx);
    bool remove_edge_batch(uint64_t ext_id1, uint16_t label, uint64_t ext_id2, Transaction& tx);

    void validate_load_graph(EdgeStream* stream, int n_threads = 1);
};