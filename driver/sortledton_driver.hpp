#ifndef SORTLEDTON_DRIVER
#define SORTLEDTON_DRIVER

#include "data-structure/TransactionManager.h"
#include "data-structure/VersioningBlockedSkipListAdjacencyList.h"

#include "driver.hpp"

class SortledtonDriver : BaseDriver {
public:
    SortledtonDriver();
    ~SortledtonDriver();
    void load_graph(EdgeStream& stream, int n_threads = 1, bool validate = false) override;
    void load_graph_batch(EdgeStream& stream, int n_threads = 1, bool validate = false);

    void update_graph(UpdateStream& update_stream, int n_threads) override;

private:
    TransactionManager tm;
    VersioningBlockedSkipListAdjacencyList* ds;

    bool add_edge(uint64_t ext_id1, uint64_t ext_id2);
    bool remove_edge(uint64_t ext_id1, uint64_t ext_id2);

    void add_edge_batch(uint64_t ext_id1, uint64_t ext_id2, SnapshotTransaction& tx);
    // bool remove_edge(uint64_t ext_id1, uint64_t ext_id2);
};

#endif