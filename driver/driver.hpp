#ifndef DRIVER
#define DRIVER

#include <atomic>

#include "../stream/stream.hpp"
#include "../stream/update_stream.hpp"

using namespace std;

class BaseDriver {
public:
    virtual void load_graph(EdgeStream& stream, int n_threads = 1, bool validate = false) = 0;
    virtual void update_graph(UpdateStream& update_stream, int n_threads) = 0;
protected:
    atomic<uint64_t> n_vertices {0};
    atomic<uint64_t> n_edges {0};
    atomic<uint64_t> updates_applied {0};
};

#endif