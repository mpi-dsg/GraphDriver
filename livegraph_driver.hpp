#include <map>
#include <atomic>

#include "tbb/concurrent_hash_map.h"
#include "livegraph/livegraph.hpp"
#include "reader/graph_reader.hpp"
#include "stream/stream.hpp"

using namespace lg;
using namespace std;

typedef tbb::concurrent_hash_map<uint64_t, uint64_t> VertexDictionary;
typedef VertexDictionary::accessor VertexDictionaryAccessor;

class LiveGraphDriver{
public:
    LiveGraphDriver();
    ~LiveGraphDriver();

    Graph* get_graph();
    void load_graph(EdgeStream* stream, int n_threads);

private:
    Graph* graph;
    map<uint64_t, uint64_t> int2ext;
    VertexDictionary ext2int;
    atomic<uint64_t> tmp_cnt {0} ;
    uint64_t check_and_insert(uint64_t& external_id, Transaction& tx);
};